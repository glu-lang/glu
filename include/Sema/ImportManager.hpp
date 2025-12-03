#ifndef GLU_SEMA_IMPORTMANAGER_HPP
#define GLU_SEMA_IMPORTMANAGER_HPP

#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"
#include "ScopeTable.hpp"

#include <llvm/Support/Allocator.h>

namespace glu::sema {

class ImportHandler;

enum class ModuleType { GluModule, IRModule, Unknown };

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
    llvm::SpecificBumpPtrAllocator<ScopeTable> _scopeTableAllocator;
    /// @brief The list of imports that were skipped due to being private.
    /// This is used to defer the processing of private imports until the
    /// end of the compilation. If linking is required, these imports will be
    /// processed so that the necessary symbols are available for linking.
    llvm::SmallVector<ast::ImportDecl *, 4> _skippedImports;

    using LocalImportResult
        = std::optional<std::tuple<ScopeTable *, llvm::StringRef>>;

    friend class ImportHandler;

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
    SourceManager *getSourceManager() const
    {
        return _context.getSourceManager();
    }
    llvm::SpecificBumpPtrAllocator<ScopeTable> &getScopeTableAllocator()
    {
        return _scopeTableAllocator;
    }

    /// @brief Get the map of imported files for linker processing
    llvm::DenseMap<FileID, ScopeTable *> const &getImportedFiles() const
    {
        return _importedFiles;
    }

    /// @brief Handles an import declaration. It is assumed that the import
    /// path is relative to the location of the import declaration.
    /// @param import The import declaration to handle.
    /// @param intoScope The scope to import the declarations into.
    /// @return Returns true if the import was successful, false otherwise.
    bool handleImport(ast::ImportDecl *import, ScopeTable *intoScope);

    /// @brief Handles the default import. It is assumed that the import
    /// path is relative to the location at the top of the import stack.
    /// @param intoScope The scope to import the declarations into.
    /// @return Returns true if the import was successful, false otherwise.
    bool handleDefaultImport(ScopeTable *intoScope);

    void addSkippedImport(ast::ImportDecl *importDecl)
    {
        _skippedImports.push_back(importDecl);
    }

    /// @brief Process all previously skipped private imports.
    /// This should be called at the end of the compilation, if linking is
    /// required.
    /// @return Returns true if all imports were successful, false otherwise.
    bool processSkippedImports();

private:
    /// @brief Tries to select a module to import from a given file.
    /// @param importLoc The source location of the import declaration, used for
    /// diagnostics.
    /// @param fid The file ID to load the module from.
    /// @param selector The selector to import (or empty to import the namespace
    /// itself, or "@all" to import all content).
    /// @return Returns an import result if it was successfully loaded, or
    /// std::nullopt if an error occurred.
    std::optional<glu::sema::ScopeTable *>
    tryLoadingFile(SourceLocation importLoc, FileID fid);
    /// @brief Detects the module type from a given file ID.
    /// @param fid The FileID of the module to detect.
    /// @return The detected ModuleType, or ModuleType::Unknown if the type
    /// could not be determined.
    ModuleType detectModuleType(FileID fid);
    /// @brief Loads a module from a file ID, using Glu, IRDec, or others.
    /// @param importLoc The source location of the import declaration, used for
    /// diagnostics.
    /// @param fid The FileID of the module to load.
    /// @param type The type of the module to load (e.g., GluModule, IRModule,
    /// Unknown).
    /// @return Returns true if the module was loaded successfully, false
    /// otherwise.
    bool loadModule(SourceLocation importLoc, FileID fid, ModuleType type);
    /// @brief Loads a module from a Glu source file.
    /// @param fid The FileID of the module to load.
    /// @return Returns true if the module was loaded successfully, false
    /// otherwise.
    bool loadGluModule(FileID fid);
    /// @brief Loads a module from an LLVM IR file.
    /// @param importLoc The source location of the import declaration, used for
    /// diagnostics.
    /// @param fid The FileID of the module to load.
    /// @return Returns true if the module was loaded successfully, false
    /// otherwise.
    bool loadIRModule(SourceLocation importLoc, FileID fid);
    /// @brief Imports a module into a given scope.
    /// @param importedModule The module to import.
    /// @param selector The selector to import (or empty to import the namespace
    /// itself, or "@all" to import all content).
    /// @param intoScope The scope to import the declarations into.
    /// @param namespaceName The name of the namespace to import the module as.
    /// @param visibility The visibility of the imported declarations.
    void importModuleIntoScope(
        SourceLocation importLoc, ScopeTable *importedModule,
        llvm::StringRef selector, ScopeTable *intoScope,
        llvm::StringRef namespaceName, ast::Visibility visibility
    );
};

} // namespace glu::sema

#endif // GLU_SEMA_IMPORTMANAGER_HPP
