#include "ASTPrinter.hpp"

namespace glu::ast {

void ASTPrinter::beforeVisit(ASTNode *node)
{
    out.indent(_indent);
    out << node->getKind() << " at loc : " << node->getLocation().getOffset();
    _indent += 2;
}

void ASTPrinter::afterVisit()
{
    _indent -= 2;
}

void ASTPrinter::visitASTNode(ASTNode *node)
{
    if (!node) {
        out.indent(_indent);
        out << "Null ASTNode\n";
    } else {
        beforeVisit(node);
        out << "\n";
        afterVisit();
    }
}

void ASTPrinter::visitIfStmt(IfStmt *node)
{
    beforeVisit(node);
    out << "\n";
    visit(node->getCondition());
    visit(node->getBody());
    visit(node->getElse());
    afterVisit();
}

void ASTPrinter::visitBreakStmt(BreakStmt *node)
{
    beforeVisit(node);
    out << "\n";
    afterVisit();
}

void ASTPrinter::visitCompoundStmt(CompoundStmt *node)
{
    beforeVisit(node);
    out << "\n";
    for (auto stmt : node->getStatements()) {
        visit(stmt);
    }
    afterVisit();
}

void ASTPrinter::visitContinueStmt(ContinueStmt *node)
{
    beforeVisit(node);
    out << "\n";
    afterVisit();
}

void ASTPrinter::visitExpressionStmt(ExpressionStmt *node)
{
    beforeVisit(node);
    out << "\n";
    if (!node->getExpr()) {
        assert(false && "ExpressionStmt has no expression.");
    }
    visit(node->getExpr());
    afterVisit();
}

void ASTPrinter::visitReturnStmt(ReturnStmt *node)
{
    beforeVisit(node);
    out << "\n";
    visit(node->getReturnExpr());
    afterVisit();
}

void ASTPrinter::visitWhileStmt(WhileStmt *node)
{
    beforeVisit(node);
    out << "\n";
    visit(node->getCondition());
    visit(node->getBody());
    afterVisit();
}

} // namespace glu::ast
