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

bool ImportManager::handleImport(
    SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
    llvm::StringRef selector, FileID ref, ScopeTable *intoScope,
    ast::Visibility visibility
)
{
    bool success = false;

    // First: determine the file to import from the components, and maybe the
    // selector. The selector can be part of the components, or it can be a
    // selector within the module. The selector can also be "*", which means
    // import all.
    llvm::SmallString<128> refDir
        = _context.getSourceManager()->getBufferName(ref);
    llvm::sys::path::remove_filename(refDir);
    if (tryImportWithin(
            importLoc, components, selector, refDir, intoScope, visibility,
            success
        )) {
        return success;
    }
    for (auto dir : _importPaths) {
        if (tryImportWithin(
                importLoc, components, selector, dir, intoScope, visibility,
                success
            )) {
            return success;
        }
    }

    return false;
}

bool ImportManager::tryImportWithin(
    SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
    llvm::StringRef selector, llvm::StringRef dir, ScopeTable *intoScope,
    ast::Visibility visibility, bool &success
)
{
    // Construct the full path to the module file.
    llvm::SmallString<128> path = dir;
    for (auto &c : components) {
        llvm::sys::path::append(path, c);
    }
    // First try with the selector as part of the path.
    // 1. ./foo/bar/baz.glu (no selector) for import foo::bar::baz;
    if (!selector.equals("*")) {
        llvm::SmallString<128> fullPath = path;
        llvm::sys::path::append(fullPath, selector + llvm::Twine(".glu"));
        // Try to import the module from the constructed path.
        if (tryImportModuleFromPath(
                importLoc, fullPath, "", selector, intoScope, visibility,
                success
            )) {
            return true;
        }
    }
    // Then try without the selector as part of the path (if no components, this
    // is invalid).
    // 2. ./foo/bar.glu (with selector baz) for import foo::bar::baz;
    if (components.empty()) {
        return false;
    }
    path += ".glu";

    // Try to import the module from the constructed path.
    return tryImportModuleFromPath(
        importLoc, path, selector, selector, intoScope, visibility, success
    );
}

bool ImportManager::tryImportModuleFromPath(
    SourceLocation importLoc, llvm::StringRef path, llvm::StringRef selector,
    llvm::StringRef namespaceName, ScopeTable *intoScope,
    ast::Visibility visibility, bool &success
)
{
    auto *sm = _context.getSourceManager();
    assert(sm && "SourceManager is required for imports");
    auto fileOrErr = sm->loadFile(path);
    if (!fileOrErr) {
        // File not found, not an error.
        return false;
    }
    FileID fid = *fileOrErr;

    if (_failedImports.contains(fid)) {
        // Previous import failed, do not try again. Do not generate new errors.
        success = false;
        return true;
    }
    if (std::find(_importStack.begin(), _importStack.end(), fid)
        != _importStack.end()) {
        // Cyclic import detected.
        _diagManager.error(
            importLoc,
            "Cyclic import detected, module imports itself indirectly"
        );
        success = false;
        return true;
    }
    if (!_importedFiles[fid]) {
        // File has not been imported yet.
        if (!loadModuleFromFileID(fid)) {
            success = false; // Import failed.
            _failedImports.insert(fid);
            return true;
        }
    }
    // File has already been imported.
    importModuleIntoScope(
        importLoc, _importedFiles[fid], selector, intoScope, namespaceName,
        visibility
    );
    success = true;
    return true;
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

void ImportManager::importModuleIntoScope(
    SourceLocation importLoc, ScopeTable *module, llvm::StringRef selector,
    ScopeTable *intoScope, llvm::StringRef namespaceName,
    ast::Visibility visibility
)
{
    if (selector.empty()) {
        // Import module as a namespace.
        intoScope->insertNamespace(namespaceName, module, visibility);
        return;
    }
    if (!module->copyInto(
            intoScope, selector, _diagManager, importLoc, visibility
        )) {
        // No elements were imported.
        _diagManager.error(
            importLoc, "Could not find '" + selector + "' in imported module"
        );
    }
}

} // namespace glu::sema
