#include "ImportManager.hpp"
#include "Sema.hpp"

#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

namespace glu::sema {

// Import Search Path Priority:
// Example: import foo::bar::baz;
// 1. ./foo/bar/baz.glu (no selector)
// 2. ./foo/bar.glu (with selector baz)
// 3. <import paths>/foo/bar/baz.glu
// 4. <import paths>/foo/bar.glu (with selector baz)
// 5. <system import paths>/foo/bar/baz.glu
// 6. <system import paths>/foo/bar.glu (with selector baz)
// Note that the system import paths are added to the import paths by the
// compiler driver, so they are not explicitly handled here.
// With wildcards, multiple files are not searched:
// Example: import foo::bar::*;
// 1. ./foo/bar.glu (with selector *)
// 2. <import paths>/foo/bar.glu (with selector *)
// 3. <system import paths>/foo/bar.glu (with selector *)
// This may be changed in the future to support wildcard imports of multiple
// files.

ImportManager::LocalImportResult ImportManager::handleImport(
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
    if (tryImportWithin(importLoc, components, selector, refDir, success)) {
        return success;
    }
    for (auto dir : _importPaths) {
        if (tryImportWithin(importLoc, components, selector, dir, success)) {
            return success;
        }
    }

    return success;
}

bool ImportManager::tryImportWithin(
    SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
    llvm::StringRef selector, llvm::StringRef dir, LocalImportResult &success
)
{
    // Construct the full path to the module file.
    llvm::SmallString<128> path = dir;
    for (auto &c : components) {
        llvm::sys::path::append(path, c);
    }
    // First try with the selector as part of the path.
    // 1. ./foo/bar/baz.glu (no selector) for import foo::bar::baz;
    if (!selector.equals("@all")) {
        llvm::SmallString<128> fullPath = path;
        llvm::sys::path::append(fullPath, selector);
        // Try to import the module from the constructed path.
        if (tryImportModuleFromPathStart(importLoc, fullPath, "", success)) {
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
    return tryImportModuleFromPathStart(importLoc, path, selector, success);
}

bool ImportManager::tryImportModuleFromPathStart(
    SourceLocation importLoc, llvm::StringRef path, llvm::StringRef selector,
    LocalImportResult &success
)
{
    auto *sm = _context.getSourceManager();
    assert(sm && "SourceManager is required for imports");

    // Try to import with the .glu extension first.
    llvm::SmallString<128> fullPath = path;
    fullPath.append(".glu");
    if (auto fileOrErr = sm->loadFile(fullPath)) {
        // Found
        success = tryImportModuleFromFile(importLoc, *fileOrErr, selector);
        return true;
    }

    // Try with .bc extension (binary LLVM bitcode).
    // fullPath = path;
    // fullPath.append(".bc");

    // No file found.
    return false;
}

std::optional<std::tuple<ScopeTable *, llvm::StringRef>>
ImportManager::tryImportModuleFromFile(
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

bool ImportManager::loadModuleFromFileID(FileID fid)
{
    _importStack.push_back(fid);
    auto *sm = _context.getSourceManager();
    glu::Scanner scanner(sm->getBuffer(fid));
    glu::Parser parser(scanner, _context, *sm, _diagManager);
    if (!parser.parse()) {
        return false;
    }
    auto *ast = llvm::cast<ast::ModuleDecl>(parser.getAST());
    if (!ast) {
        return false;
    }
    _importedFiles[fid] = sema::fastConstrainAST(ast, _diagManager, this);
    assert(_importStack.back() == fid);
    _importStack.pop_back();
    return _importedFiles[fid] != nullptr;
}

static bool isOperatorOverload(llvm::StringRef name)
{
    return llvm::StringSwitch<bool>(name)
#define OPERATOR(Name, Symbol, Type) .Case(Symbol, true)
#include "Basic/TokenKind.def"
        .Case("[", true)
        .Default(false);
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
    std::function<bool(llvm::StringRef)> selectorFunc;
    if (selector == "@all") {
        selectorFunc = [](llvm::StringRef) { return true; };
    } else {
        selectorFunc
            = [selector](llvm::StringRef name) { return name == selector; };
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
        auto [loc, path] = _skippedImports.back();
        _skippedImports.pop_back();
        if (!handleImport(loc, path, nullptr, ast::Visibility::Private)) {
            return false;
        }
    }
    return true;
}

} // namespace glu::sema
