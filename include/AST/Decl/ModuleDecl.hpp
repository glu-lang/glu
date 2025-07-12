#ifndef GLU_AST_DECL_MODULEDECL_HPP
#define GLU_AST_DECL_MODULEDECL_HPP

#include "ASTContext.hpp"
#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

namespace glu::ast {

/// @class ModuleDecl
/// @brief Represents a module declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a module
/// declaration.
class ModuleDecl final : public DeclBase,
                         private llvm::TrailingObjects<ModuleDecl, DeclBase *> {
    llvm::StringRef _name;
    ASTContext *_ctx;

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        ModuleDecl, _numDecls, DeclBase *, Decls
    )

    ModuleDecl(
        SourceLocation location, llvm::StringRef name,
        llvm::ArrayRef<DeclBase *> decls, ASTContext *ctx
    )
        : DeclBase(NodeKind::ModuleDeclKind, location, nullptr)
        , _name(name)
        , _ctx(ctx)
    {
        initDecls(decls);
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
        llvm::StringRef name, llvm::ArrayRef<DeclBase *> decls, ASTContext *ctx
    )
    {
        void *mem = alloc.Allocate(
            totalSizeToAlloc<DeclBase *>(decls.size()), alignof(ModuleDecl)
        );

        return new (mem) ModuleDecl(location, name, decls, ctx);
    }

    /// @brief Getter for the name of the module.
    /// @return Returns the name of the module.
    llvm::StringRef getName() const { return _name; }

    SourceManager *getSourceManager() const { return _ctx->getSourceManager(); }

    ASTContext *getContext() const { return _ctx; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ModuleDeclKind;
    }
};
}

#endif // GLU_AST_DECL_MODULEDECL_HPP
