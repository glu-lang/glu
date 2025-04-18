#ifndef GLU_AST_EXPR_LITERAL_EXPR_HPP
#define GLU_AST_EXPR_LITERAL_EXPR_HPP

#include "ASTNode.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <variant>

namespace glu::ast {

/// @brief Represents a literal expression in the AST (e.g., 42, 3.14, "abc",
/// true).
class LiteralExpr : public ExprBase {
public:
    using LiteralValue
        = std::variant<llvm::APInt, llvm::APFloat, llvm::StringRef, bool>;

private:
    LiteralValue _value;

public:
    /// @brief Constructs a LiteralExpr.
    /// @param value the value of the literal
    /// @param type the type of the literal (should be a primitive type matching
    /// the value)
    /// @param loc the source location of the literal token
    LiteralExpr(
        LiteralValue value, glu::types::TypeBase *type, SourceLocation loc
    )
        : ExprBase(NodeKind::LiteralExprKind, loc), _value(value)
    {
        setType(type);
    }

    /// @brief Returns the value of the literal, as a variant.
    LiteralValue getValue() { return _value; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::LiteralExprKind;
    }
};

}

#endif // GLU_AST_EXPR_LITERAL_EXPR_HPP
