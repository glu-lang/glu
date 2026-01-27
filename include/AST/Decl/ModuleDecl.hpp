#ifndef GLU_AST_DECL_MODULEDECL_HPP
#define GLU_AST_DECL_MODULEDECL_HPP

#include "ASTContext.hpp"
#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"
#include "Decl/DeclBase.hpp"

namespace glu::ast {

/// @class ModuleDecl
/// @brief Represents a module declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a module
/// declaration.
class ModuleDecl final : public DeclBase,
                         private llvm::TrailingObjects<ModuleDecl, DeclBase *> {
    llvm::StringRef _filepath;
    llvm::StringRef _importName;
    ASTContext *_ctx;
    bool _isIRDecModule = false;

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        ModuleDecl, _numDecls, DeclBase *, Decls
    )

    ModuleDecl(
        SourceLocation location, llvm::ArrayRef<DeclBase *> decls,
        ASTContext *ctx
    )
        : DeclBase(
              NodeKind::ModuleDeclKind, location, nullptr, Visibility::Public
          )
        , _ctx(ctx)
    {
        initDecls(decls);
        if (getSourceManager()) {
            _filepath = getSourceManager()->getBufferName(location);
            _importName = getSourceManager()->getImportName(_filepath);
        }
    }

public:
    /// @brief Static method to create é new ModuleDecl.
    /// @param alloc The allocator to use for memory allocation.
    /// @param location The source location of the module declaration.
    /// @param name The name of the module.
    /// @param decls The declarations within the module.
    /// @return Returns a pointer to the newly created ModuleDecl.
    static ModuleDecl *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location,
        llvm::ArrayRef<DeclBase *> decls, ASTContext *ctx
    )
    {
        void *mem = alloc.Allocate(
            totalSizeToAlloc<DeclBase *>(decls.size()), alignof(ModuleDecl)
        );

        return new (mem) ModuleDecl(location, decls, ctx);
    }

    /// @brief Getter for the import name of the module. This is the full name
    /// of the module, stripped of the user path. For example, for a module
    /// whose file path is "/Users/me/projects/glutalk/communication.glu", the
    /// import name would be "glutalk/communication". The name of at least one
    /// parent directory is always included to avoid name clashes, although it
    /// is probably not included in this way.
    /// @return Returns the import name of the module.
    llvm::StringRef getImportName() const { return _importName; }

    /// @brief Getter for the path to the file of the module. For files that
    /// are not loaded from a file, this will be an empty string.
    /// @return Returns the file path of the module.
    llvm::StringRef getFilePath() const { return _filepath; }

    SourceManager *getSourceManager() const { return _ctx->getSourceManager(); }

    ASTContext *getContext() const { return _ctx; }

    bool isIRDecModule() const { return _isIRDecModule; }
    ModuleDecl *markAsIRDecModule()
    {
        _isIRDecModule = true;
        return this;
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ModuleDeclKind;
    }

    /// @brief Get all declarations of a specific type.
    /// @tparam DeclType The type of declarations to retrieve.
    /// @return Returns an array reference to declarations of the specified
    /// type.
    template <typename DeclType>
    llvm::SmallVector<DeclType *> getDeclsOfType() const
    {
        llvm::SmallVector<DeclType *, 4> results;
        for (auto *decl : getDecls()) {
            if (auto *casted = llvm::dyn_cast<DeclType>(decl)) {
                results.push_back(casted);
            }
        }
        return results;
    }
};
}

#endif // GLU_AST_DECL_MODULEDECL_HPP
