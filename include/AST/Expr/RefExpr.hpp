#ifndef GLU_AST_EXPR_REFEXPR_HPP
#define GLU_AST_EXPR_REFEXPR_HPP

#include "ASTNode.hpp"
#include "Decls.hpp"

#include <llvm/ADT/PointerUnion.h>

namespace glu::ast {

/// @brief Represents a reference expression in the AST.
class RefExpr : public ExprBase {
    using ReferencedVarDecl = llvm::PointerUnion<VarLetDecl, FunctionDecl>;

    /// @brief The name of the reference.
    llvm::StringRef _name;
    /// @brief The variable declaration that this reference refers to.
    ReferencedVarDecl _variable;

public:
    /// @brief Constructor for RefExpr.
    /// @param loc The source location of the reference.
    /// @param name The name of the reference.
    /// @param variable The variable declaration that this reference refers to.
    /// @note The name is stored as a StringRef, which means it should be
    ///       valid for the lifetime of the RefExpr.
    RefExpr(SourceLocation loc, llvm::StringRef name)
        : ExprBase(NodeKind::RefExprKind, loc), _name(name), _variable(nullptr)
    {
    }

    /// @brief Get the name of the reference.
    /// @return The name of the reference.
    llvm::StringRef getName() const { return _name; }

    /// @brief Get the variable declaration that this reference refers to.
    /// @return The variable declaration that this reference refers to.
    ReferencedVarDecl getVariable() const { return _variable; }

    /// @brief Set the variable declaration that this reference refers to.
    /// @param variable The variable declaration that this reference refers to.
    void setVariable(ReferencedVarDecl variable) { _variable = variable; }

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
