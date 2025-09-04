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
    llvm::DenseMap<FileID, ast::ModuleDecl *> _importedFiles;
    /// @brief A set of FileIDs that have previously failed to import.
    /// This set is used to avoid repeated attempts to import files that have
    /// already failed to import. If a file is in this set, it will not be
    /// attempted to be imported again and will be ignored.
    llvm::DenseSet<FileID> _failedImports;

public:
    ImportManager(ast::ASTContext &context, DiagnosticManager &diagManager)
        : _context(context), _diagManager(diagManager)
    {
        _importStack.push_back(context.getSourceManager()->getMainFileID());
    }

    /// @brief Handles an import declaration. It is assumed that the import
    /// path is relative to the current file (the top of the import stack).
    /// @param path The import path to handle.
    /// @param intoScope The scope to import the declarations into.
    /// @return Returns true if the import was successful, false otherwise.
    bool handleImport(ast::ImportPath path, ScopeTable *intoScope)
    {
        bool success = true;
        assert(
            !_importStack.empty()
            && "Import stack should never be empty when handling imports"
        );
        for (auto selector : path.selectors) {
            if (!handleImport(
                    path.components, selector, _importStack.back(), intoScope
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
        llvm::ArrayRef<llvm::StringRef> components, llvm::StringRef selector,
        FileID ref, ScopeTable *intoScope
    );
};

} // namespace glu::sema

#endif // GLU_SEMA_IMPORTMANAGER_HPP
