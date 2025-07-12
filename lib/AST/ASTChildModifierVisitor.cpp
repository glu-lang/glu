#include "ASTChildModifierVisitor.hpp"

namespace glu::ast {

// Implementation of visitor methods
void ASTChildModifierVisitor::visitVarLetDecl(
    VarLetDecl *node, llvm::StringRef name, types::TypeBase *type,
    ExprBase *value
)
{
    node->setName(name);
    node->setType(type);
    node->setValue(value);
}

void ASTChildModifierVisitor::visitAssignStmt(
    AssignStmt *node, ExprBase *left, Token op, ExprBase *right
)
{
    node->setExprLeft(left);
    node->setOperator(op);
    node->setExprRight(right);
}

void ASTChildModifierVisitor::visitDeclStmt(DeclStmt *node, DeclBase *decl)
{
    node->setDecl(decl);
}

void ASTChildModifierVisitor::visitExpressionStmt(
    ExpressionStmt *node, ExprBase *expr
)
{
    node->setExpr(expr);
}

void ASTChildModifierVisitor::visitReturnStmt(ReturnStmt *node, ExprBase *expr)
{
    node->setReturnExpr(expr);
}

void ASTChildModifierVisitor::visitIfStmt(
    IfStmt *node, ExprBase *condition, CompoundStmt *body,
    CompoundStmt *elseBranch
)
{
    node->setCondition(condition);
    node->setBody(body);
    node->setElse(elseBranch);
}

void ASTChildModifierVisitor::visitForStmt(
    ForStmt *node, ForBindingDecl *binding, ExprBase *range, CompoundStmt *body
)
{
    node->setBinding(binding);
    node->setRange(range);
    node->setBody(body);
}

void ASTChildModifierVisitor::visitWhileStmt(
    WhileStmt *node, ExprBase *condition, CompoundStmt *body
)
{
    node->setCondition(condition);
    node->setBody(body);
}

void ASTChildModifierVisitor::visitCompoundStmt(
    CompoundStmt *node, llvm::ArrayRef<StmtBase *> stmts
)
{
    node->setStmts(stmts);
}

void ASTChildModifierVisitor::visitCallExpr(
    CallExpr *node, ExprBase *callee, llvm::ArrayRef<ExprBase *> args
)
{
    node->setCallee(callee);
    node->setArgs(args);
}

void ASTChildModifierVisitor::visitTernaryConditionalExpr(
    TernaryConditionalExpr *node, ExprBase *condition, ExprBase *trueExpr,
    ExprBase *falseExpr
)
{
    node->setCondition(condition);
    node->setTrueExpr(trueExpr);
    node->setFalseExpr(falseExpr);
}

void ASTChildModifierVisitor::visitUnaryOpExpr(
    UnaryOpExpr *node, ExprBase *value, RefExpr *op
)
{
    node->setOperand(value);
    node->setOperator(op);
}

void ASTChildModifierVisitor::visitBinaryOpExpr(
    BinaryOpExpr *node, ExprBase *leftOperand, RefExpr *op,
    ExprBase *rightOperand
)
{
    node->setLeftOperand(leftOperand);
    node->setOperator(op);
    node->setRightOperand(rightOperand);
}

void ASTChildModifierVisitor::visitCastExpr(
    CastExpr *node, ExprBase *value, types::TypeBase *destType
)
{
    node->setCastedExpr(value);
    node->setDestType(destType);
}

void ASTChildModifierVisitor::visitStructMemberExpr(
    StructMemberExpr *node, ExprBase *structExpr, llvm::StringRef memberName
)
{
    node->setStructExpr(structExpr);
    node->setMemberName(memberName);
}

void ASTChildModifierVisitor::visitDynamicArrayTy(
    types::DynamicArrayTy *node, types::TypeBase *dataType
)
{
    node->setDataType(dataType);
}

void ASTChildModifierVisitor::visitUnresolvedNameTy(
    types::UnresolvedNameTy *node, llvm::StringRef name
)
{
    node->setName(name);
}

// Static method to replace a child expression in its parent
void ASTChildModifierVisitor::replaceChildExpr(
    ExprBase *oldExpr, ExprBase *newExpr
)
{
    auto *parent = oldExpr->getParent();
    if (!parent)
        return;

    // Use direct casting instead of visitor
    if (auto *exprStmt = llvm::dyn_cast<ast::ExpressionStmt>(parent)) {
        if (exprStmt->getExpr() == oldExpr) {
            exprStmt->setExpr(newExpr);
        }
    } else if (auto *returnStmt = llvm::dyn_cast<ast::ReturnStmt>(parent)) {
        if (returnStmt->getReturnExpr() == oldExpr) {
            returnStmt->setReturnExpr(newExpr);
        }
    } else if (auto *assignStmt = llvm::dyn_cast<ast::AssignStmt>(parent)) {
        if (assignStmt->getExprLeft() == oldExpr) {
            assignStmt->setExprLeft(newExpr);
        } else if (assignStmt->getExprRight() == oldExpr) {
            assignStmt->setExprRight(newExpr);
        }
    } else if (auto *ifStmt = llvm::dyn_cast<ast::IfStmt>(parent)) {
        if (ifStmt->getCondition() == oldExpr) {
            ifStmt->setCondition(newExpr);
        }
    } else if (auto *whileStmt = llvm::dyn_cast<ast::WhileStmt>(parent)) {
        if (whileStmt->getCondition() == oldExpr) {
            whileStmt->setCondition(newExpr);
        }
    } else if (auto *forStmt = llvm::dyn_cast<ast::ForStmt>(parent)) {
        if (forStmt->getRange() == oldExpr) {
            forStmt->setRange(newExpr);
        }
    } else if (auto *callExpr = llvm::dyn_cast<ast::CallExpr>(parent)) {
        if (callExpr->getCallee() == oldExpr) {
            callExpr->setCallee(newExpr);
        } else {
            // Check in the arguments
            auto args = callExpr->getArgs();
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] == oldExpr) {
                    // Replace the argument at index i
                    llvm::SmallVector<ExprBase *, 8> newArgs(
                        args.begin(), args.end()
                    );
                    newArgs[i] = newExpr;
                    callExpr->setArgs(newArgs);
                    break;
                }
            }
        }
    } else if (auto *ternaryExpr
               = llvm::dyn_cast<ast::TernaryConditionalExpr>(parent)) {
        if (ternaryExpr->getCondition() == oldExpr) {
            ternaryExpr->setCondition(newExpr);
        } else if (ternaryExpr->getTrueExpr() == oldExpr) {
            ternaryExpr->setTrueExpr(newExpr);
        } else if (ternaryExpr->getFalseExpr() == oldExpr) {
            ternaryExpr->setFalseExpr(newExpr);
        }
    } else if (auto *unaryExpr = llvm::dyn_cast<ast::UnaryOpExpr>(parent)) {
        if (unaryExpr->getOperand() == oldExpr) {
            unaryExpr->setOperand(newExpr);
        }
    } else if (auto *binaryExpr = llvm::dyn_cast<ast::BinaryOpExpr>(parent)) {
        if (binaryExpr->getLeftOperand() == oldExpr) {
            binaryExpr->setLeftOperand(newExpr);
        } else if (binaryExpr->getRightOperand() == oldExpr) {
            binaryExpr->setRightOperand(newExpr);
        }
    } else if (auto *castExpr = llvm::dyn_cast<ast::CastExpr>(parent)) {
        if (castExpr->getCastedExpr() == oldExpr) {
            castExpr->setCastedExpr(newExpr);
        }
    } else if (auto *structMemberExpr
               = llvm::dyn_cast<ast::StructMemberExpr>(parent)) {
        if (structMemberExpr->getStructExpr() == oldExpr) {
            structMemberExpr->setStructExpr(newExpr);
        }
    }

    // Update parent-child relationships
    newExpr->setParent(parent);
}

// Implementations of expression replacement methods
void ASTChildModifierVisitor::visitExpressionStmt(
    ExpressionStmt *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getExpr() == oldExpr) {
        node->setExpr(newExpr);
    }
}

void ASTChildModifierVisitor::visitReturnStmt(
    ReturnStmt *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getReturnExpr() == oldExpr) {
        node->setReturnExpr(newExpr);
    }
}

void ASTChildModifierVisitor::visitAssignStmt(
    AssignStmt *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getExprLeft() == oldExpr) {
        node->setExprLeft(newExpr);
    } else if (node->getExprRight() == oldExpr) {
        node->setExprRight(newExpr);
    }
}

void ASTChildModifierVisitor::visitIfStmt(
    IfStmt *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getCondition() == oldExpr) {
        node->setCondition(newExpr);
    }
}

void ASTChildModifierVisitor::visitWhileStmt(
    WhileStmt *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getCondition() == oldExpr) {
        node->setCondition(newExpr);
    }
}

void ASTChildModifierVisitor::visitForStmt(
    ForStmt *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getRange() == oldExpr) {
        node->setRange(newExpr);
    }
}

void ASTChildModifierVisitor::visitCallExpr(
    CallExpr *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getCallee() == oldExpr) {
        node->setCallee(newExpr);
    } else {
        // Check in the arguments
        auto args = node->getArgs();
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == oldExpr) {
                // Replace the argument at index i
                llvm::SmallVector<ExprBase *, 8> newArgs(
                    args.begin(), args.end()
                );
                newArgs[i] = newExpr;
                node->setArgs(newArgs);
                break;
            }
        }
    }
}

void ASTChildModifierVisitor::visitTernaryConditionalExpr(
    TernaryConditionalExpr *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getCondition() == oldExpr) {
        node->setCondition(newExpr);
    } else if (node->getTrueExpr() == oldExpr) {
        node->setTrueExpr(newExpr);
    } else if (node->getFalseExpr() == oldExpr) {
        node->setFalseExpr(newExpr);
    }
}

void ASTChildModifierVisitor::visitUnaryOpExpr(
    UnaryOpExpr *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getOperand() == oldExpr) {
        node->setOperand(newExpr);
    }
}

void ASTChildModifierVisitor::visitBinaryOpExpr(
    BinaryOpExpr *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getLeftOperand() == oldExpr) {
        node->setLeftOperand(newExpr);
    } else if (node->getRightOperand() == oldExpr) {
        node->setRightOperand(newExpr);
    }
}

void ASTChildModifierVisitor::visitCastExpr(
    CastExpr *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getCastedExpr() == oldExpr) {
        node->setCastedExpr(newExpr);
    }
}

void ASTChildModifierVisitor::visitStructMemberExpr(
    StructMemberExpr *node, ExprBase *oldExpr, ExprBase *newExpr
)
{
    if (node->getStructExpr() == oldExpr) {
        node->setStructExpr(newExpr);
    }
}

} // namespace glu::ast
