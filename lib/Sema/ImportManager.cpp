#include "ImportManager.hpp"

namespace glu::sema {

// Import Search Path Priority:
// Example: import foo::bar::baz;
// 1. ./foo/bar/baz.glu (no selector)
// 2. ./foo/bar.glu (with selector baz)
// 3. <import paths>/foo/bar/baz.glu
// 4. <import paths>/foo/bar.glu (with selector baz)
// 5. <system import paths>/foo/bar/baz.glu
// 6. <system import paths>/foo/bar.glu (with selector baz)
// With wildcards, multiple files are not searched:
// Example: import foo::bar::*;
// 1. ./foo/bar.glu (with selector *)
// 2. <import paths>/foo/bar.glu (with selector *)
// 3. <system import paths>/foo/bar.glu (with selector *)
// This may be changed in the future to support wildcard imports of multiple
// files.

bool ImportManager::handleImport(
    llvm::ArrayRef<llvm::StringRef> components, llvm::StringRef selector,
    FileID ref, ScopeTable *intoScope
)
{
    bool success = false;

    // First: determine the file to import from the components, and maybe the
    // selector. The selector can be part of the components, or it can be a
    // selector within the module. The selector can also be "*", which means
    // import all.
    if (tryImportWithin(
            components, selector,
            _context.getSourceManager()->getDirectoryName(ref), intoScope,
            success
        )) {
        return success;
    }
    for (auto dir : _importPaths) {
        if (tryImportWithin(components, selector, dir, intoScope, success)) {
            return success;
        }
    }
    if (tryImportWithin(
            components, selector,
            _context.getSourceManager()->getDirectoryName(ref), intoScope,
            success
        )) {
        return success;
    }

    return false;
}

bool ImportManager::tryImportWithin(
    llvm::ArrayRef<llvm::StringRef> components, llvm::StringRef selector,
    llvm::StringRef dir, ScopeTable *intoScope, bool &success
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
        if (tryImportModuleFromPath(fullPath, "", intoScope, success)) {
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
    return tryImportModuleFromPath(path, selector, intoScope, success);
}

bool ImportManager::tryImportModuleFromPath(
    llvm::StringRef path, llvm::StringRef selector, ScopeTable *intoScope,
    bool &success
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
            sm->getLocForStartOfFile(fid), // FIXME: use import loc
            "Cyclic import detected, module imports itself indirectly"
        );
        success = false;
        return true;
    }
    llvm::errs() << "Importing module from file: " << path << "\n";
    if (!_importedFiles[fid]) {
        // File has not been imported yet.
        if (!loadModuleFromFileID(fid)) {
            success = false; // Import failed.
            return true;
        }
    }
    // File has already been imported.
    importModuleIntoScope(_importedFiles[fid], selector, intoScope);
    success = true;
    return true;
}

bool ImportManager::loadModuleFromFileID(FileID fid)
{
    return true;
}

void ImportManager::importModuleIntoScope(
    ScopeTable *module, llvm::StringRef selector, ScopeTable *intoScope
)
{
}

} // namespace glu::sema
