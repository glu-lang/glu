#ifndef GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP
#define GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP

#include "ASTNode.hpp"
#include "Types.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <variant>

namespace glu::ast {

class LiteralExpr : public ExprBase {
    using LiteralValue
        = std::variant<llvm::APInt, llvm::APFloat, llvm::StringRef, bool>;
    LiteralValue _value;
    glu::types::TypeBase *_type;

public:
    LiteralExpr(
        LiteralValue value, glu::types::TypeBase *type, SourceLocation loc
    )
        : ExprBase(NodeKind::LiteralExprKind, loc), _value(value), _type(type)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::LiteralExprKind;
    }

    glu::types::TypeBase *getType() { return _type; }

    LiteralValue getValue() { return _value; }
};

}

#endif // GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP
