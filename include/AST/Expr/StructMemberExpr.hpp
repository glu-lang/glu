#ifndef GLU_AST_EXPR_STRUCT_MEMBER_EXPR_KIND_HPP
#define GLU_AST_EXPR_STRUCT_MEMBER_EXPR_KIND_HPP

#include "ASTNode.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::ast {

class StructMemberExpr : public ExprBase {
    ExprBase *_value;
    llvm::StringRef _memberName;

public:
    StructMemberExpr(
        SourceLocation loc, ExprBase *value, llvm::StringRef memberName
    )
        : ExprBase(NodeKind::StructMemberExprKind, loc)
        , _value(value)
        , _memberName(memberName)
    {
    }

    ExprBase *getStructExpr() const { return _value; }

    llvm::StringRef getMemberName() const { return _memberName; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::StructMemberExprKind;
    }
};

}

#endif // GLU_AST_EXPR_STRUCT_MEMBER_EXPR_KIND_HPP
