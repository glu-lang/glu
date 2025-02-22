#include "ASTPrinter.hpp"

namespace glu::ast {

void ASTPrinter::beforeVisit(ASTNode *node)
{
    out.indent(_indent);
    out << node->getKind()
        << " at file : " << _srcManager->getBufferName(node->getLocation())
        << " line : " << _srcManager->getSpellingLineNumber(node->getLocation())
        << " col : "
        << _srcManager->getSpellingColumnNumber(node->getLocation());
    _indent += 2;
}

void ASTPrinter::afterVisit()
{
    _indent -= 2;
}

/// @brief Visits an AST node.
/// @param node The AST node to be visited.
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

/// @brief Visits an IfStmt node.
/// @param node The IfStmt node to be visited.
void ASTPrinter::visitIfStmt(IfStmt *node)
{
    beforeVisit(node);
    out << "\n";
    visit(node->getCondition());
    visit(node->getBody());
    visit(node->getElse());
    afterVisit();
}

/// @brief Visits a BreakStmt node.
/// @param node The BreakStmt node to be visited.
void ASTPrinter::visitBreakStmt(BreakStmt *node)
{
    beforeVisit(node);
    out << "\n";
    afterVisit();
}

/// @brief Visits a CompoundStmt node.
/// @param node The CompoundStmt node to be visited.
void ASTPrinter::visitCompoundStmt(CompoundStmt *node)
{
    beforeVisit(node);
    out << "\n";
    for (auto stmt : node->getStatements()) {
        visit(stmt);
    }
    afterVisit();
}

/// @brief Visits a ContinueStmt node.
/// @param node The ContinueStmt node to be visited.
void ASTPrinter::visitContinueStmt(ContinueStmt *node)
{
    beforeVisit(node);
    out << "\n";
    afterVisit();
}

/// @brief Visits an ExpressionStmt node.
/// @param node The ExpressionStmt node to be visited.
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

/// @brief Visits a ReturnStmt node.
/// @param node The ReturnStmt node to be visited.
void ASTPrinter::visitReturnStmt(ReturnStmt *node)
{
    beforeVisit(node);
    out << "\n";
    visit(node->getReturnExpr());
    afterVisit();
}

/// @brief Visits a WhileStmt node.
/// @param node The WhileStmt node to be visited.
void ASTPrinter::visitWhileStmt(WhileStmt *node)
{
    beforeVisit(node);
    out << "\n";
    visit(node->getCondition());
    visit(node->getBody());
    afterVisit();
}

} // namespace glu::ast
