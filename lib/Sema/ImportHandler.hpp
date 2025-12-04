#ifndef GLU_SEMA_IMPORTHANDLER_HPP
#define GLU_SEMA_IMPORTHANDLER_HPP

#include "ImportManager.hpp"

namespace glu::sema {

/// @brief The resolved file and selector path for an import.
struct ResolvedFileImport {
    /// @brief The FileID of the imported file.
    FileID _fileID;
    /// @brief The selector path used for the import.
    llvm::ArrayRef<llvm::StringRef> selectorPath;
};

/// @brief The resolved scope and selector for an import.
struct ResolvedImport {
    /// The scope table of the imported module.
    ScopeTable *scope;
    /// @brief The selector used for the import (empty if importing the
    /// namespace, or "@all" if importing all content).
    llvm::StringRef selector;
};

/// @brief Responsible for handling a single import declaration path. Holds the
/// state for the import process. Does not handle multiple imports, multiple
/// paths, or the action of applying the resolved import into a scope.
class ImportHandler {
    /// @brief The global import manager.
    ImportManager &_manager;
    /// @brief The import declaration being handled.
    /// May be nullptr if the import is implicit (e.g. standard library).
    ast::ImportDecl *_importDecl;
    /// @brief The FileID of the file performing the import.
    FileID _importingFileID;
    /// @brief The import path being processed.
    llvm::SmallVector<llvm::StringRef> _path;

    SourceLocation getImportLocation() const
    {
        if (_importDecl) {
            return _importDecl->getLocation();
        } else {
            return SourceLocation::invalid;
        }
    }

public:
    /// @brief Constructs an ImportHandler for a given non-null ImportDecl.
    /// @param manager The global import manager.
    /// @param importDecl The import declaration being handled.
    /// @param selector The last import path component.
    ImportHandler(
        ImportManager &manager, ast::ImportDecl *importDecl,
        llvm::StringRef selector
    )
        : _manager(manager)
        , _importDecl(importDecl)
        , _importingFileID(
              manager.getSourceManager()->getFileID(importDecl->getLocation())
          )
    {
        auto importPath = importDecl->getImportPath().components;
        _path.assign(importPath.begin(), importPath.end());
        _path.push_back(selector);
    }

    /// @brief Constructs an ImportHandler for an implicit import (no
    /// ImportDecl).
    /// @param manager The global import manager.
    /// @param importingFileID The FileID of the file performing the import.
    /// @param path The full import path (including selector).
    ImportHandler(
        ImportManager &manager, FileID importingFileID,
        llvm::ArrayRef<llvm::StringRef> path
    )
        : _manager(manager)
        , _importDecl(nullptr)
        , _importingFileID(importingFileID)
        , _path(path.begin(), path.end())
    {
    }

    /// @brief Processes the import path and resolves it to a module scope and
    /// selector.
    /// @return The resolved import information if successful, std::nullopt
    /// otherwise.
    std::optional<ResolvedImport> resolveImport();

private:
    /// @brief Processes the import path and resolves it to a module scope and
    /// selector.
    /// @return The resolved import information if successful, std::nullopt
    /// otherwise.
    std::optional<ResolvedFileImport> resolveFileImport();
    /// @brief Resolves an import path using set extensions.
    /// @param extensions The list of file extensions to try when resolving
    /// the import.
    /// @return The resolved import information if successful, std::nullopt
    /// otherwise.
    std::optional<ResolvedFileImport>
    resolveImportWithExtensions(llvm::ArrayRef<llvm::StringRef> extensions);
    /// @brief Resolves an import at a given base directory with set
    /// extensions.
    /// @param baseDir The base directory to resolve the import from.
    /// @param extensions The list of file extensions to try when resolving
    /// the import.
    /// @return The resolved import information if successful, std::nullopt
    /// otherwise.
    std::optional<ResolvedFileImport> resolveImportAtPath(
        llvm::StringRef baseDir, llvm::ArrayRef<llvm::StringRef> extensions
    );
    /// @brief Resolves an import at a given base directory with a given
    /// number of path components and set extensions.
    /// @param baseDir The base directory to resolve the import from.
    /// @param components The number of path components to use for the module
    /// file.
    /// @param extensions The list of file extensions to try when resolving
    /// the import.
    /// @return The resolved import information if successful, std::nullopt
    /// otherwise.
    std::optional<ResolvedFileImport> resolveImportWithComponents(
        llvm::StringRef baseDir, size_t components,
        llvm::ArrayRef<llvm::StringRef> extensions
    );
    std::optional<ResolvedImport> loadModule(ResolvedFileImport resolvedFile);
};

} // namespace glu

#endif // GLU_SEMA_IMPORTHANDLER_HPP
