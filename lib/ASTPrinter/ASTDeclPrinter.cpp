#include "ASTPrinter.hpp"

namespace glu::ast {

/// @brief Visits an EnumDecl node.
/// @param node The EnumDecl node to be visited.
void ASTPrinter::visitEnumDecl(EnumDecl *node)
{
    beforeVisit(node);
    out << "\nName: " << node->getName() << "; Members : ";

    size_t caseCount = node->getType()->getCaseCount();
    for (size_t i = 0; i < caseCount; ++i) {
        out << node->getType()->getCase(i);
        if (i + 1 < caseCount) {
            out << ", ";
        }
    }
    out << "\n";
    afterVisit();
}

/// @brief Visits a FunctionDecl node.
/// @param node The FunctionDecl node to be visited.
void ASTPrinter::visitFunctionDecl(FunctionDecl *node)
{
    beforeVisit(node);
    out << "\n";
    visit(&node->getBody());
    afterVisit();
}

/// @brief Visits a LetDecl node.
/// @param node The LetDecl node to be visited.
void ASTPrinter::visitLetDecl(LetDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    visit(node->getValue());
    afterVisit();
}

/// @brief Visits a StructDecl node.
/// @param node The StructDecl node to be visited.
void ASTPrinter::visitStructDecl(StructDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "; Fields : ";
    size_t fieldCount = node->getType()->getFieldCount();
    for (size_t i = 0; i < fieldCount; ++i) {
        out << node->getType()->getField(i);
        if (i + 1 < fieldCount) {
            out << ", ";
        }
    }
    out << "\n";
    afterVisit();
}

/// @brief Visits a TypeAliasDecl node.
/// @param node The TypeAliasDecl node to be visited.
void ASTPrinter::visitTypeAliasDecl(TypeAliasDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    afterVisit();
}

/// @brief Visits a VarDecl node.
/// @param node The VarDecl node to be visited.
void ASTPrinter::visitVarDecl(VarDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    visit(node->getValue());
    afterVisit();
}

/// @brief Visits a VarLetDecl node.
/// @param node The VarLetDecl node to be visited.
void ASTPrinter::visitVarLetDecl(VarLetDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    visit(node->getValue());
    afterVisit();
}

} // namespace glu::ast
