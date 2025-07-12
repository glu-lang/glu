#ifndef GLU_AST_DECL_MODULEDECL_HPP
#define GLU_AST_DECL_MODULEDECL_HPP

#include "ASTContext.hpp"
#include "ASTNode.hpp"

namespace glu::ast {

/// @class ModuleDecl
/// @brief Represents a module declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a module
/// declaration.
class ModuleDecl final : public DeclBase,
                         private llvm::TrailingObjects<ModuleDecl, DeclBase *> {
    using TrailingParams = llvm::TrailingObjects<ModuleDecl, DeclBase *>;
    friend TrailingParams;

    llvm::StringRef _name;
    unsigned _numDecls;
    ASTContext *_ctx;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(
        typename TrailingParams::OverloadToken<DeclBase *>
    ) const
    {
        return _numDecls;
    }

    ModuleDecl(
        SourceLocation location, llvm::StringRef name,
        llvm::ArrayRef<DeclBase *> decls, ASTContext *ctx
    )
        : DeclBase(NodeKind::ModuleDeclKind, location, nullptr)
        , _name(name)
        , _numDecls(decls.size())
        , _ctx(ctx)
    {
        std::uninitialized_copy(
            decls.begin(), decls.end(), getTrailingObjects<DeclBase *>()
        );
        for (unsigned i = 0; i < _numDecls; i++) {
            getTrailingObjects<DeclBase *>()[i]->setParent(this);
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

    /// @brief Getter for the declarations within the module.
    /// @return Returns a vector of declarations within the module.
    llvm::ArrayRef<DeclBase *> getDecls() const
    {
        return { getTrailingObjects<DeclBase *>(), _numDecls };
    }

    SourceManager *getSourceManager() const { return _ctx->getSourceManager(); }

    /// @brief Set the declarations within the module.
    /// @param decls A vector of declarations to set within the module.
    void setDecls(llvm::ArrayRef<DeclBase *> decls)
    {
        _numDecls = decls.size();
        std::uninitialized_copy(
            decls.begin(), decls.end(), getTrailingObjects<DeclBase *>()
        );
        for (unsigned i = 0; i < _numDecls; i++) {
            getTrailingObjects<DeclBase *>()[i]->setParent(this);
        }
    }

    ASTContext *getContext() const { return _ctx; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ModuleDeclKind;
    }
};

}

#endif // GLU_AST_DECL_MODULEDECL_HPP
