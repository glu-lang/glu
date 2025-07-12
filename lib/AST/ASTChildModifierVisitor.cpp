#ifndef GLU_AST_CHILD_MODIFIER_VISITOR_HPP
#define GLU_AST_CHILD_MODIFIER_VISITOR_HPP

#include "ASTVisitor.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {

// Ce visiteur modifie les enfants des noeuds AST via les setters
class ASTChildModifierVisitor : public ASTVisitor<ASTChildModifierVisitor> {
public:

    void visitVarLetDecl(
        VarLetDecl *node, llvm::StringRef name, types::TypeBase *type,
        ExprBase *value
    )
    {
        node->setName(name);
        node->setType(type);
        node->setValue(value);
    }

    void
    visitAssignStmt(AssignStmt *node, ExprBase *left, Token op, ExprBase *right)
    {
        node->setExprLeft(left);
        node->setOperator(op);
        node->setExprRight(right);
    }

    void visitDeclStmt(DeclStmt *node, DeclBase *decl) { node->setDecl(decl); }

    void visitExpressionStmt(ExpressionStmt *node, ExprBase *expr)
    {
        node->setExpr(expr);
    }

    void visitReturnStmt(ReturnStmt *node, ExprBase *expr)
    {
        node->setReturnExpr(expr);
    }

    void visitIfStmt(
        IfStmt *node, ExprBase *condition, CompoundStmt *body,
        CompoundStmt *elseBranch
    )
    {
        node->setCondition(condition);
        node->setBody(body);
        node->setElse(elseBranch);
    }

    void visitForStmt(
        ForStmt *node, ForBindingDecl *binding, ExprBase *range,
        CompoundStmt *body
    )
    {
        node->setBinding(binding);
        node->setRange(range);
        node->setBody(body);
    }

    void
    visitWhileStmt(WhileStmt *node, ExprBase *condition, CompoundStmt *body)
    {
        node->setCondition(condition);
        node->setBody(body);
    }

    void visitCompoundStmt(CompoundStmt *node, llvm::ArrayRef<StmtBase *> stmts)
    {
        node->setStmts(stmts);
    }

    void visitCallExpr(
        CallExpr *node, ExprBase *callee, llvm::ArrayRef<ExprBase *> args
    )
    {
        node->setCallee(callee);
        node->setArgs(args);
    }

    void visitTernaryConditionalExpr(
        TernaryConditionalExpr *node, ExprBase *condition, ExprBase *trueExpr,
        ExprBase *falseExpr
    )
    {
        node->setCondition(condition);
        node->setTrueExpr(trueExpr);
        node->setFalseExpr(falseExpr);
    }

    void visitUnaryOpExpr(UnaryOpExpr *node, ExprBase *value, RefExpr *op)
    {
        node->setOperand(value);
        node->setOperator(op);
    }

    void
    visitDynamicArrayTy(types::DynamicArrayTy *node, types::TypeBase *dataType)
    {
        node->setDataType(dataType);
    }

    void
    visitUnresolvedNameTy(types::UnresolvedNameTy *node, llvm::StringRef name)
    {
        node->setName(name);
    }

};

template<typename... Args>
void modifyChildren(ASTNode *node, Args&&... args) {
    ASTChildModifierVisitor visitor;
    visitor.visit(node, std::forward<Args>(args)...);
}


} // namespace glu::ast

#endif // GLU_AST_CHILD_MODIFIER_VISITOR_HPP
