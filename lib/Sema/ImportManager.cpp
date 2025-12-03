#include "ImportManager.hpp"
#include "Sema.hpp"

#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

#include "IRDec/ModuleLifter.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>

namespace glu::sema {

// Import Search Path Priority:
// Example: import foo::bar::baz;
// 1. ./foo/bar/baz.glu (no selector) (Glu source file)
// 2. ./foo/bar/baz.bc (no selector) (decompiling LLVM bitcode)
// 3. ./foo/bar/baz.ll (no selector) (human-readable LLVM IR)
// 4. ./foo/bar.glu (with selector baz) (Glu source file)
// 5. ./foo/bar.bc (with selector baz) (decompiling LLVM bitcode)
// 6. ./foo/bar.ll (with selector baz) (human-readable LLVM IR)
// 7. <import paths>/foo/bar/baz.glu (no selector) (Glu source file)
// 8. <import paths>/foo/bar/baz.bc (no selector) (decompiling LLVM bitcode)
// 9. <import paths>/foo/bar/baz.ll (no selector) (human-readable LLVM IR)
// 10. <import paths>/foo/bar.glu (with selector baz) (Glu source file)
// 11. <import paths>/foo/bar.bc (with selector baz) (decompiling LLVM bitcode)
// 12. <import paths>/foo/bar.ll (with selector baz) (human-readable LLVM IR)
// Note that the system import paths are added at the end of the import paths by
// the compiler driver, so they are not explicitly handled here. With wildcards,
// files are not wildcard-expanded, the wildcard is treated as a selector.
// Example: import foo::bar::*;
// 1. ./foo/bar.glu (with selector *)
// 2. <import paths>/foo/bar.glu (with selector *)
// 3. <system import paths>/foo/bar.glu (with selector *)
// This may be changed in the future to support wildcard imports of multiple
// files. Decompiling bitcode or LLVM IR with wildcards is also supported with
// wildcards, but not shown here for brevity.

// MARK: - Import Resolution

ImportManager::LocalImportResult ImportManager::findImport(
    SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
    llvm::StringRef selector, FileID ref
)
{
    LocalImportResult success = std::nullopt;

    // First: determine the file to import from the components, and maybe the
    // selector. The selector can be part of the components, or it can be a
    // selector within the module. The selector can also be "@all", which means
    // import all.
    llvm::SmallString<128> refDir
        = _context.getSourceManager()->getBufferName(ref);
    llvm::sys::path::remove_filename(refDir);
    if (trySelectDirectory(importLoc, components, selector, refDir, success)) {
        return success;
    }
    for (auto dir : _importPaths) {
        if (trySelectDirectory(importLoc, components, selector, dir, success)) {
            return success;
        }
    }

    return success;
}

bool ImportManager::trySelectDirectory(
    SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
    llvm::StringRef selector, llvm::StringRef dir, LocalImportResult &result
)
{
    // Construct the full path to the module file.
    llvm::SmallString<128> path = dir;
    for (auto &c : components) {
        llvm::sys::path::append(path, c);
    }
    // First try with the selector as part of the path.
    // 1. ./foo/bar/baz.glu (no selector) for import foo::bar::baz;
    if (selector != "@all") {
        llvm::SmallString<128> fullPath = path;
        llvm::sys::path::append(fullPath, selector);
        // Try to import the module from the constructed path.
        if (trySelectPath(importLoc, fullPath, "", result)) {
            return true;
        }
    }
    // Then try without the selector as part of the path (if no components, this
    // is invalid).
    // 2. ./foo/bar.glu (with selector baz) for import foo::bar::baz;
    if (components.empty()) {
        return false;
    }

    // Try to import the module from the constructed path.
    return trySelectPath(importLoc, path, selector, result);
}

bool ImportManager::trySelectPath(
    SourceLocation importLoc, llvm::StringRef path, llvm::StringRef selector,
    LocalImportResult &result
)
{
    auto *sm = _context.getSourceManager();
    assert(sm && "SourceManager is required for imports");

    // Try to import with the .glu extension first.
    llvm::SmallString<128> fullPath = path;
    fullPath.append(".glu");
    if (auto fileOrErr = sm->loadFile(fullPath)) {
        // Found
        result = tryLoadingFile(importLoc, *fileOrErr, selector);
        return true;
    }

    // Try with .bc extension (binary LLVM bitcode).
    fullPath = path;
    fullPath.append(".bc");
    if (auto fileOrErr = sm->loadIRFile(fullPath)) {
        // Found
        result = tryLoadingIRFile(importLoc, *fileOrErr, selector);
        return true;
    }

    // Try with .ll extension (human-readable LLVM IR).
    fullPath = path;
    fullPath.append(".ll");
    if (auto fileOrErr = sm->loadIRFile(fullPath)) {
        // Found
        result = tryLoadingIRFile(importLoc, *fileOrErr, selector);
        return true;
    }

    // No file found.
    return false;
}

// MARK: - Import File Loading

ImportManager::LocalImportResult ImportManager::tryLoadingFile(
    SourceLocation importLoc, FileID fid, llvm::StringRef selector
)
{
    if (_failedImports.contains(fid)) {
        // Previous import failed, do not try again. Do not generate new errors.
        return std::nullopt;
    }
    if (std::find(_importStack.begin(), _importStack.end(), fid)
        != _importStack.end()) {
        // Cyclic import detected.
        _diagManager.error(
            importLoc,
            "Cyclic import detected, module may be re-exporting itself"
        );
        return std::nullopt;
    }
    if (!_importedFiles[fid]) {
        // File has not been imported yet.
        if (!loadModuleFromFileID(fid)) {
            _failedImports.insert(fid);
            return std::nullopt; // Import failed.
        }
    }
    // File has already been imported.
    return std::tuple(_importedFiles[fid], selector);
}

ImportManager::LocalImportResult ImportManager::tryLoadingIRFile(
    SourceLocation importLoc, FileID fid, llvm::StringRef selector
)
{
    if (_failedImports.contains(fid)) {
        // Previous import failed, do not try again. Do not generate new errors.
        return std::nullopt;
    }
    if (!_importedFiles[fid]) {
        // File has not been imported yet.
        if (!loadModuleFromIRFile(importLoc, fid)) {
            _failedImports.insert(fid);
            return std::nullopt; // Import failed.
        }
    }
    // File has already been imported.
    return std::tuple(_importedFiles[fid], selector);
}

bool ImportManager::loadModuleFromFileID(FileID fid)
{
    auto *sm = _context.getSourceManager();
    glu::Scanner scanner(sm->getBuffer(fid), _context.getScannerAllocator());
    glu::Parser parser(scanner, _context, *sm, _diagManager);
    if (!parser.parse()) {
        return false;
    }
    auto *ast = llvm::cast<ast::ModuleDecl>(parser.getAST());
    if (!ast) {
        return false;
    }
    _importStack.push_back(fid);
    _importedFiles[fid] = sema::fastConstrainAST(ast, _diagManager, this);
    assert(_importStack.back() == fid);
    _importStack.pop_back();
    return _importedFiles[fid] != nullptr;
}

bool ImportManager::loadModuleFromIRFile(SourceLocation importLoc, FileID fid)
{
    llvm::SMDiagnostic err;
    llvm::LLVMContext localContext;
    auto *sm = _context.getSourceManager();
    auto llvmModule
        = llvm::parseIRFile(sm->getBufferName(fid), err, localContext);

    if (!llvmModule) {
        _diagManager.error(
            importLoc, "Failed to parse LLVM module: " + err.getMessage()
        );
        return false;
    }

    auto *ast = glu::irdec::liftModule(_context, llvmModule.get());

    _importStack.push_back(fid);
    _importedFiles[fid] = sema::fastConstrainAST(ast, _diagManager, this);
    assert(_importStack.back() == fid);
    _importStack.pop_back();
    return _importedFiles[fid] != nullptr;
}

// MARK: - Module Copying

static llvm::StringRef isOperatorOverload(llvm::StringRef name)
{
    return llvm::StringSwitch<bool>(name)
#define OPERATOR(Name, Symbol, Type) .Case(Symbol, true)
#include "Basic/TokenKind.def"
               .Case("[", true)
               .Case("begin", true)
               .Case("end", true)
               .Case("next", true)
               .Default(false)
        ? name
        : "";
}

void ImportManager::importModuleIntoScope(
    SourceLocation importLoc, ScopeTable *module, llvm::StringRef selector,
    ScopeTable *intoScope, llvm::StringRef namespaceName,
    ast::Visibility visibility
)
{
    if (selector.empty()) {
        // Import module as a namespace.
        intoScope->insertNamespace(namespaceName, module, visibility);
        // Copy operator overloads into the scope directly.
        module->copyInto(
            intoScope, isOperatorOverload, _diagManager, importLoc, visibility
        );
        return;
    }
    // Import selected items from the module.
    std::function<llvm::StringRef(llvm::StringRef)> selectorFunc;
    if (selector == "@all") {
        selectorFunc = [](llvm::StringRef name) { return name; };
    } else {
        selectorFunc = [selector, namespaceName](llvm::StringRef name) {
            return name == selector ? namespaceName : "";
        };
    }
    if (!module->copyInto(
            intoScope, selectorFunc, _diagManager, importLoc, visibility
        )) {
        // No elements were imported.
        _diagManager.error(
            importLoc, "Could not find '" + selector + "' in imported module"
        );
    }
}

bool ImportManager::processSkippedImports()
{
    while (!_skippedImports.empty()) {
        auto *decl = _skippedImports.back();
        _skippedImports.pop_back();
        if (!handleImport(
                decl->getLocation(), decl->getImportPath(), nullptr,
                ast::Visibility::Private
            )) {
            return false;
        }
    }
    return true;
}

} // namespace glu::sema
