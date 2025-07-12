#ifndef GLU_AST_EXPR_CAST_EXPR_HPP
#define GLU_AST_EXPR_CAST_EXPR_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

namespace glu::ast {

/// @brief Represents a cast expression in the AST (e.g., x as UInt8).
class CastExpr : public ExprBase {
    ExprBase *_value;
    glu::types::TypeBase *_destType;

public:
    /// @brief Constructs a CastExpr.
    /// @param loc the location of the cast keyword
    /// @param value the expression to be casted
    /// @param destType the type to cast the expression to
    CastExpr(
        SourceLocation loc, ExprBase *value, glu::types::TypeBase *destType
    )
        : ExprBase(NodeKind::CastExprKind, loc)
        , _value(value)
        , _destType(destType)
    {
        assert(value && "Value cannot be null.");
        assert(destType && "Destination type cannot be null.");
        value->setParent(this);
    }

    /// @brief Returns the expression to be casted.
    ExprBase *getCastedExpr() const { return _value; }

    /// @brief Sets the expression to be casted.
    /// @param value the new expression to cast
    void setCastedExpr(ExprBase *value)
    {
        if (_value != nullptr) {
            _value->setParent(nullptr);
        }
        _value = value;
        if (value)
            value->setParent(this);
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
