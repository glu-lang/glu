#ifndef GLU_AST_EXPR_REFEXPR_HPP
#define GLU_AST_EXPR_REFEXPR_HPP

#include "ASTNode.hpp"
#include "Decls.hpp"

#include <llvm/ADT/PointerUnion.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @brief Represents a reference expression in the AST using trailing objects
/// to store both the name and the variable declaration.
class RefExpr final
    : public ExprBase,
      private llvm::TrailingObjects<
          RefExpr, llvm::PointerUnion<VarLetDecl, FunctionDecl>> {
public:
    using ReferencedVarDecl = llvm::PointerUnion<VarLetDecl, FunctionDecl>;

private:
    using TrailingArgs = llvm::TrailingObjects<RefExpr, ReferencedVarDecl>;
    llvm::StringRef _name;
    friend TrailingArgs;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename TrailingArgs::OverloadToken<ReferencedVarDecl>) const
    {
        return 1;
    }

    RefExpr(
        SourceLocation loc, llvm::StringRef name, ReferencedVarDecl variable
    )
        : ExprBase(NodeKind::RefExprKind, loc), _name(name)
    {
        setVariable(variable);
    }

public:
    /// @brief Constructs a RefExpr object. (to be called via the memory arena)
    /// @param allocator The allocator of the memory arena (internal)
    /// @param loc the location of the reference expression
    /// @param name the name of the reference expression
    /// @param variable the variable declaration referenced by this reference
    /// @return the newly created RefExpr object
    static RefExpr *create(
        llvm::BumpPtrAllocator &allocator, SourceLocation loc,
        llvm::StringRef name, ReferencedVarDecl variable = nullptr
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<ReferencedVarDecl>(1), alignof(RefExpr)
        );
        return new (mem) RefExpr(loc, name, variable);
    }

    /// @brief Get the name of the reference.
    /// @return The name of the reference.
    llvm::StringRef getName() const { return _name; }

    /// @brief Get the variable declaration that this reference refers to.
    /// @return The variable declaration that this reference refers to.
    ReferencedVarDecl getVariable() const
    {
        return getTrailingObjects<ReferencedVarDecl>()[0];
    }

    /// @brief Set the variable declaration that this reference refers to.
    /// @param variable The variable declaration that this reference refers to.
    void setVariable(ReferencedVarDecl variable)
    {
        getTrailingObjects<ReferencedVarDecl>()[0] = variable;
    }

    /// @brief Check if the node is a RefExpr.
    /// @param node The node to check.
    /// @return True if the node is a RefExpr, false otherwise.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::RefExprKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_EXPR_REFEXPR_HPP
