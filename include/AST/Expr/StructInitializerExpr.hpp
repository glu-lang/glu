#ifndef GLU_AST_EXPR_STRUCTINITIALIZER_HPP
#define GLU_AST_EXPR_STRUCTINITIALIZER_HPP

#include "ASTNode.hpp"

namespace glu::ast {

class StructInitializerExpr final
    : public ExprBase,
      private llvm::TrailingObjects<StructInitializerExpr, ExprBase *> {

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        StructInitializerExpr, _argCount, ExprBase *, Fields
    )

public:
    StructInitializerExpr(SourceLocation loc, llvm::ArrayRef<ExprBase *> fields)
        : ExprBase(NodeKind::StructInitializerExprKind, loc)
    {
        initFields(fields);
    }

    static StructInitializerExpr *create(
        llvm::BumpPtrAllocator &allocator, SourceLocation loc,
        llvm::ArrayRef<ExprBase *> fields
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<ExprBase *>(fields.size()),
            alignof(StructInitializerExpr)
        );
        return new (mem) StructInitializerExpr(loc, fields);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::StructInitializerExprKind;
    }
};

}

#endif // GLU_AST_EXPR_STRUCTINITIALIZER_HPP
