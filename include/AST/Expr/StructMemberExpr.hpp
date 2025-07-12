#ifndef GLU_AST_EXPR_STRUCT_MEMBER_EXPR_HPP
#define GLU_AST_EXPR_STRUCT_MEMBER_EXPR_HPP

#include "ASTNode.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::ast {

/// @brief Represents a struct member access expression in the AST (e.g.,
/// val.member).
class StructMemberExpr : public ExprBase {
    ExprBase *_value;
    llvm::StringRef _memberName;

public:
    /// @brief Constructs a StructMemberExpr.
    /// @param loc the source location of the dot token
    /// @param value the value, of a struct type, to access the member from
    /// @param memberName the name of the member to access
    StructMemberExpr(
        SourceLocation loc, ExprBase *value, llvm::StringRef memberName
    )
        : ExprBase(NodeKind::StructMemberExprKind, loc)
        , _value(value)
        , _memberName(memberName)
    {
        assert(value && "Value cannot be null.");
        value->setParent(this);
    }

    /// @brief Returns the expression representing the struct.
    ExprBase *getStructExpr() const { return _value; }

    /// @brief Sets the expression representing the struct.
    /// @param value the new struct expression
    void setStructExpr(ExprBase *value)
    {
        if (_value != nullptr) {
            _value->setParent(nullptr);
        }
        _value = value;
        if (value)
            value->setParent(this);
    }

    /// @brief Returns the name of the member to access.
    llvm::StringRef getMemberName() const { return _memberName; }

    /// @brief Sets the name of the member to access.
    /// @param memberName the new member name
    void setMemberName(llvm::StringRef memberName) { _memberName = memberName; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::StructMemberExprKind;
    }
};

}

#endif // GLU_AST_EXPR_STRUCT_MEMBER_EXPR_HPP
