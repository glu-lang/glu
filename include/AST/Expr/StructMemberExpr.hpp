#ifndef GLU_AST_EXPR_STRUCT_MEMBER_EXPR_HPP
#define GLU_AST_EXPR_STRUCT_MEMBER_EXPR_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::ast {

/// @brief Represents a struct member access expression in the AST (e.g.,
/// val.member).
class StructMemberExpr : public ExprBase {

    GLU_AST_GEN_CHILD(StructMemberExpr, ExprBase *, _value, StructExpr)

private:
    llvm::StringRef _memberName;

public:
    /// @brief Constructs a StructMemberExpr.
    /// @param loc the source location of the dot token
    /// @param value the value, of a struct type, to access the member from
    /// @param memberName the name of the member to access
    StructMemberExpr(
        SourceLocation loc, ExprBase *value, llvm::StringRef memberName
    )
        : ExprBase(NodeKind::StructMemberExprKind, loc), _memberName(memberName)
    {
        initStructExpr(value);
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
