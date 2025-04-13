#ifndef GLU_AST_DECL_MODULEDECL_HPP
#define GLU_AST_DECL_MODULEDECL_HPP

#include "ASTNode.hpp"
#include "Basic/SourceManager.hpp"

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
    glu::SourceManager *_sm;

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
        llvm::ArrayRef<DeclBase *> decls, glu::SourceManager *sm
    )
        : DeclBase(NodeKind::ModuleDeclKind, location, nullptr)
        , _name(name)
        , _numDecls(decls.size())
        , _sm(sm)
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
        llvm::StringRef name, llvm::ArrayRef<DeclBase *> decls,
        glu::SourceManager *sm
    )
    {
        void *mem = alloc.Allocate(
            totalSizeToAlloc<DeclBase *>(decls.size()), alignof(ModuleDecl)
        );

        return new (mem) ModuleDecl(location, name, decls, sm);
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

    SourceManager *getSourceManager() const { return _sm; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ModuleDeclKind;
    }
};

}

#endif // GLU_AST_DECL_MODULEDECL_HPP
