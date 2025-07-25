#ifndef GLU_AST_EXPR_CAST_EXPR_HPP
#define GLU_AST_EXPR_CAST_EXPR_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"
#include "Types.hpp"

namespace glu::ast {

/// @brief Represents a cast expression in the AST (e.g., x as UInt8).
class CastExpr : public ExprBase {

    GLU_AST_GEN_CHILD(CastExpr, ExprBase *, _value, CastedExpr)

private:
    glu::types::TypeBase *_destType;

public:
    /// @brief Constructs a CastExpr.
    /// @param loc the location of the cast keyword
    /// @param value the expression to be casted
    /// @param destType the type to cast the expression to
    CastExpr(
        SourceLocation loc, ExprBase *value, glu::types::TypeBase *destType
    )
        : ExprBase(NodeKind::CastExprKind, loc), _destType(destType)
    {
        initCastedExpr(value);
        assert(destType && "Destination type cannot be null.");
    }

    /// @brief Returns the type to cast the expression to.
    glu::types::TypeBase *getDestType() const { return _destType; }

    /// @brief Sets the type to cast the expression to.
    /// @param type the new destination type for the cast expression
    void setDestType(glu::types::TypeBase *type) { _destType = type; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::CastExprKind;
    }
};

}

#endif // GLU_AST_EXPR_CAST_EXPR_HPP
