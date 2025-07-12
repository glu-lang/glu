#ifndef GLU_AST_CHILD_MODIFIER_VISITOR_HPP
#define GLU_AST_CHILD_MODIFIER_VISITOR_HPP

#include "ASTVisitor.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {

class ASTChildModifierVisitor : public ASTVisitor<ASTChildModifierVisitor> {
public:
    // Declarations of visit methods
    void visitVarLetDecl(
        VarLetDecl *node, llvm::StringRef name, types::TypeBase *type,
        ExprBase *value
    );
    void visitAssignStmt(
        AssignStmt *node, ExprBase *left, Token op, ExprBase *right
    );
    void visitDeclStmt(DeclStmt *node, DeclBase *decl);
    void visitExpressionStmt(ExpressionStmt *node, ExprBase *expr);
    void visitReturnStmt(ReturnStmt *node, ExprBase *expr);
    void visitIfStmt(
        IfStmt *node, ExprBase *condition, CompoundStmt *body,
        CompoundStmt *elseBranch
    );
    void visitForStmt(
        ForStmt *node, ForBindingDecl *binding, ExprBase *range,
        CompoundStmt *body
    );
    void
    visitWhileStmt(WhileStmt *node, ExprBase *condition, CompoundStmt *body);
    void
    visitCompoundStmt(CompoundStmt *node, llvm::ArrayRef<StmtBase *> stmts);
    void visitCallExpr(
        CallExpr *node, ExprBase *callee, llvm::ArrayRef<ExprBase *> args
    );
    void visitTernaryConditionalExpr(
        TernaryConditionalExpr *node, ExprBase *condition, ExprBase *trueExpr,
        ExprBase *falseExpr
    );
    void visitUnaryOpExpr(UnaryOpExpr *node, ExprBase *value, RefExpr *op);
    void visitBinaryOpExpr(
        BinaryOpExpr *node, ExprBase *leftOperand, RefExpr *op,
        ExprBase *rightOperand
    );
    void
    visitCastExpr(CastExpr *node, ExprBase *value, types::TypeBase *destType);
    void visitStructMemberExpr(
        StructMemberExpr *node, ExprBase *structExpr, llvm::StringRef memberName
    );
    void
    visitDynamicArrayTy(types::DynamicArrayTy *node, types::TypeBase *dataType);
    void
    visitUnresolvedNameTy(types::UnresolvedNameTy *node, llvm::StringRef name);

    // Method to replace a child expression in its parent
    static void replaceChildExpr(ExprBase *oldExpr, ExprBase *newExpr);

    // Method to replace a specific child expression in a node
    void visitExpressionStmt(
        ExpressionStmt *node, ExprBase *oldExpr, ExprBase *newExpr
    );
    void
    visitReturnStmt(ReturnStmt *node, ExprBase *oldExpr, ExprBase *newExpr);
    void
    visitAssignStmt(AssignStmt *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitIfStmt(IfStmt *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitWhileStmt(WhileStmt *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitForStmt(ForStmt *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitCallExpr(CallExpr *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitTernaryConditionalExpr(
        TernaryConditionalExpr *node, ExprBase *oldExpr, ExprBase *newExpr
    );
    void
    visitUnaryOpExpr(UnaryOpExpr *node, ExprBase *oldExpr, ExprBase *newExpr);
    void
    visitBinaryOpExpr(BinaryOpExpr *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitCastExpr(CastExpr *node, ExprBase *oldExpr, ExprBase *newExpr);
    void visitStructMemberExpr(
        StructMemberExpr *node, ExprBase *oldExpr, ExprBase *newExpr
    );
};

// Fonction template pour modifier les enfants de n'importe quel noeud
template <typename... Args> void modifyChildren(ASTNode *node, Args &&...args)
{
    ASTChildModifierVisitor visitor;
    visitor.visit(node, std::forward<Args>(args)...);
}

} // namespace glu::ast

#endif // GLU_AST_CHILD_MODIFIER_VISITOR_HPP
