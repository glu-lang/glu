#include "ImportHandler.hpp"
#include "Exprs.hpp"

// Import Search Path Priority:
// Example: import foo::bar::baz;
// 1. ./foo.glu (selector: bar::baz)
// 2. ./foo/bar.glu (selector: baz)
// 3. ./foo/bar/baz.glu (no selector)
// 4. <import paths>/foo.glu (selector: bar::baz)
// 5. <import paths>/foo/bar.glu (selector: baz)
// 6. <import paths>/foo/bar/baz.glu (no selector)
// 7. Repeat the above with .h for C header files.
// 8. Repeat the above with .bc and .ll extensions for LLVM bitcode and
//    human-readable LLVM IR files respectively.
// 9. Repeat the above with .c, .cpp, .rs, etc. for foreign language source
// files
//    that can be imported directly using IRDec.
// Note that the system import paths are added at the end of the import paths by
// the compiler driver, so they are not explicitly handled here. With wildcards,
// files are not wildcard-expanded, the wildcard is treated as a selector.
// Example: import foo::bar::*;
// 1. ./foo.glu (selector: bar::*)
// 2. ./foo/bar.glu (selector: *)
// 3. <import paths>/foo.glu (with selector bar::*)
// 4. <import paths>/foo/bar.glu (with selector *)
// This may be changed in the future to support wildcard imports of multiple
// files. Decompiling bitcode or LLVM IR with wildcards is also supported with
// wildcards, but not shown here for brevity.

namespace glu::sema {

std::optional<ResolvedImport> ImportHandler::resolveImport()
{
    auto fileImportOpt = resolveFileImport();
    if (!fileImportOpt) {
        // File not found
        return std::nullopt;
    }
    return loadModule(*fileImportOpt);
}

std::optional<ResolvedFileImport> ImportHandler::resolveFileImport()
{
    // 4 levels:
    // 1. Glu source files (.glu), always preferred
    // 2. C/C++ header files (.h), using ClangImporter
    // 3. LLVM IR files (.ll/.bc), using IRDec
    // 4. Foreign source files (.c/.cpp/.rs/etc.), using its compiler + IRDec
    llvm::SmallVector<llvm::StringRef> const supportedExtensions[] = {
        { ".glu" },
        { ".h" },
        { ".bc", ".ll" },
        { ".c", ".cpp", ".cc", ".cxx", ".C", ".rs", ".zig", ".swift" },
    };

    if (_importDecl) {
        if (auto *ext = _importDecl->getAttribute(
                ast::AttributeKind::FileExtensionKind
            )) {
            auto *literal
                = llvm::dyn_cast<ast::LiteralExpr>(ext->getParameter());
            if (literal
                && std::holds_alternative<llvm::StringRef>(
                    literal->getValue()
                )) {
                auto fileExt = std::get<llvm::StringRef>(literal->getValue());
                return resolveImportWithExtensions({ fileExt });
            }
        }
    }

    for (auto const &extensions : supportedExtensions) {
        if (auto result = resolveImportWithExtensions(extensions)) {
            return result;
        }
    }
    return std::nullopt;
}

std::optional<ResolvedFileImport> ImportHandler::resolveImportWithExtensions(
    llvm::ArrayRef<llvm::StringRef> extensions
)
{
    // Try all search paths
    llvm::SmallString<128> refDir
        = _manager.getSourceManager()->getBufferName(_importingFileID);
    llvm::sys::path::remove_filename(refDir);
    if (auto result = resolveImportAtPath(refDir, extensions)) {
        return result;
    }

    for (auto dir : _manager._importPaths) {
        if (auto result = resolveImportAtPath(dir, extensions)) {
            return result;
        }
    }
    return std::nullopt;
}

std::optional<ResolvedFileImport> ImportHandler::resolveImportAtPath(
    llvm::StringRef baseDir, llvm::ArrayRef<llvm::StringRef> extensions
)
{
    // Example: import foo::bar::baz;
    // 1. baseDir/foo.ext (selector: bar::baz) - take 1 component
    // 2. baseDir/foo/bar.ext (selector: baz) - take 2 components
    // 3. baseDir/foo/bar/baz.ext (no selector) - take 3 (all) components

    for (size_t i = 1; i <= _path.size(); ++i) {
        if (auto result = resolveImportWithComponents(baseDir, i, extensions)) {
            return result;
        }
    }
    return std::nullopt;
}

std::optional<ResolvedFileImport> ImportHandler::resolveImportWithComponents(
    llvm::StringRef baseDir, size_t components,
    llvm::ArrayRef<llvm::StringRef> extensions
)
{
    llvm::SmallString<128> path = baseDir;
    for (size_t i = 0; i < components; ++i) {
        if (_path[i] == "@all") {
            return std::nullopt; // Invalid to have @all in path components
        }
        llvm::sys::path::append(path, _path[i]);
    }
    for (auto ext : extensions) {
        llvm::sys::path::replace_extension(path, ext);
        // Try to load the file, without loading its content yet
        auto fid = _manager.getSourceManager()->loadFile(path, false);
        if (fid) {
            if (*fid == _importingFileID) {
                // Skip self-imports (try next options)
                continue;
            }
            return ResolvedFileImport {
                *fid, llvm::ArrayRef(_path).slice(components)
            };
        }
    }
    return std::optional<ResolvedFileImport>();
}

std::optional<ResolvedImport> ImportHandler::loadModule(ResolvedFileImport file)
{
    auto scope = _manager.tryLoadingFile(getImportLocation(), file._fileID);
    if (!scope) {
        return std::nullopt;
    }
    auto selectorPath = file.selectorPath;
    if (selectorPath.empty()) {
        // Importing the namespace itself
        return ResolvedImport { *scope, "" };
    }
    while (selectorPath.size() > 1) {
        auto nextScope = (*scope)->lookupNamespace(selectorPath.front());
        if (!nextScope) {
            _manager.getDiagnosticManager().error(
                getImportLocation(),
                "Module has no namespace named '" + selectorPath.front() + "'"
            );
            return std::nullopt;
        }
        scope = nextScope;
        selectorPath = selectorPath.slice(1);
    }
    assert(selectorPath.size() == 1 && "Selector path must have one element");
    return ResolvedImport { *scope, selectorPath.back() };
}

} // namespace glu::sema
