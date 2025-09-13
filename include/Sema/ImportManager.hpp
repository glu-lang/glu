#ifndef GLU_SEMA_IMPORTMANAGER_HPP
#define GLU_SEMA_IMPORTMANAGER_HPP

#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"
#include "ScopeTable.hpp"

namespace glu::sema {

/// @brief The ImportManager class is responsible for handling import
/// declarations in the AST. It is able to detect cyclic imports and report
/// errors for invalid import paths.
class ImportManager {
    ast::ASTContext &_context;
    DiagnosticManager &_diagManager;
    /// @brief The stack of imported files to detect cyclic imports.
    /// This stack contains the FileIDs of the files that are currently being
    /// imported. If a file is encountered that is already in the stack, a
    /// cyclic import is detected.
    llvm::SmallVector<FileID, 8> _importStack;
    /// @brief A map from FileID to the corresponding ModuleDecl.
    /// This map is used to keep track of which files have already been imported
    /// and their corresponding ModuleDecls. It acts as a cache to avoid
    /// re-importing files that have already been processed.
    llvm::DenseMap<FileID, ScopeTable *> _importedFiles;
    /// @brief A set of FileIDs that have previously failed to import.
    /// This set is used to avoid repeated attempts to import files that have
    /// already failed to import. If a file is in this set, it will not be
    /// attempted to be imported again and will be ignored.
    llvm::DenseSet<FileID> _failedImports;
    /// @brief The import paths to search for imported files.
    /// This list contains the directories that will be searched when
    /// attempting to resolve import paths. The directories are searched in
    /// the order they are listed in this array.
    /// The order is:
    /// 1. The directory of the file that is doing the import (the top of the
    ///    import stack).
    /// 2. The directories specified in this array.
    /// 3. The system import paths (where standard library modules are
    ///    located).
    llvm::ArrayRef<std::string> _importPaths;
    /// @brief Allocator for scope tables created during imports.
    llvm::BumpPtrAllocator _scopeTableAllocator;

public:
    ImportManager(
        ast::ASTContext &context, DiagnosticManager &diagManager,
        llvm::ArrayRef<std::string> importPaths
    )
        : _context(context)
        , _diagManager(diagManager)
        , _importPaths(importPaths)
    {
        if (context.getSourceManager()) {
            _importStack.push_back(context.getSourceManager()->getMainFileID());
        } // else, imports are invalid
    }

    DiagnosticManager &getDiagnosticManager() { return _diagManager; }
    ast::ASTContext &getASTContext() const { return _context; }
    llvm::BumpPtrAllocator &getScopeTableAllocator()
    {
        return _scopeTableAllocator;
    }

    /// @brief Get the map of imported files for linker processing
    llvm::DenseMap<FileID, ScopeTable *> const &getImportedFiles() const
    {
        return _importedFiles;
    }

    /// @brief Handles an import declaration. It is assumed that the import
    /// path is relative to the current file (the top of the import stack).
    /// @param path The import path to handle.
    /// @param intoScope The scope to import the declarations into.
    /// @return Returns true if the import was successful, false otherwise.
    bool handleImport(
        SourceLocation importLoc, ast::ImportPath path, ScopeTable *intoScope,
        ast::Visibility visibility
    )
    {
        bool success = true;
        assert(
            !_importStack.empty()
            && "Import stack should never be empty when handling imports"
        );
        for (auto selector : path.selectors) {
            if (!handleImport(
                    importLoc, path.components, selector, _importStack.back(),
                    intoScope, visibility
                )) {
                success = false;
            }
        }

        return success;
    }

private:
    /// @brief Handles a single import selector. It is assumed that the import
    /// path is relative to the current file (the top of the import stack).
    /// @param components The components of the import path.
    /// @param selector The selector to import.
    /// @param ref The FileID of the file from which the import is being made.
    /// @param intoScope The scope to import the declarations into.
    /// @return Returns true if the import was successful, false otherwise.
    bool handleImport(
        SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
        llvm::StringRef selector, FileID ref, ScopeTable *intoScope,
        ast::Visibility visibility
    );
    /// @brief Tries to import a module from a given directory.
    /// @param components The components of the import path.
    /// @param selector The selector to import.
    /// @param dir The directory to search for the module.
    /// @param intoScope The scope to import the declarations into.
    /// @param error Set to true if an error occurred during import. Not
    /// modified if the file was not found.
    /// @return Returns true if the file to import was found, false otherwise.
    bool tryImportWithin(
        SourceLocation importLoc, llvm::ArrayRef<llvm::StringRef> components,
        llvm::StringRef selector, llvm::StringRef dir, ScopeTable *intoScope,
        ast::Visibility visibility, bool &error
    );
    /// @brief Tries to import a module from a given path.
    /// @param path The full path to the module file (including the extension).
    /// @param selector The selector to import (or empty to import the namespace
    /// itself, or "*" to import all content).
    /// @param intoScope The scope to import the declarations into.
    /// @param error Set to true if an error occurred during import. Not
    /// modified if the file was not found.
    /// @return Returns true if the file to import was found, false otherwise.
    bool tryImportModuleFromPath(
        SourceLocation importLoc, llvm::StringRef path,
        llvm::StringRef selector, llvm::StringRef namespaceName,
        ScopeTable *intoScope, ast::Visibility visibility, bool &error
    );
    bool loadModuleFromFileID(FileID fid);
    void importModuleIntoScope(
        SourceLocation importLoc, ScopeTable *importedModule,
        llvm::StringRef selector, ScopeTable *intoScope,
        llvm::StringRef namespaceName, ast::Visibility visibility
    );
};

} // namespace glu::sema

#endif // GLU_SEMA_IMPORTMANAGER_HPP
