#include "ASTPrinter.hpp"

namespace glu::ast {

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

void ASTPrinter::visitFunctionDecl(FunctionDecl *node)
{
    beforeVisit(node);
    out << "\n";
    visit(&node->getBody());
    afterVisit();
}

void ASTPrinter::visitLetDecl(LetDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    visit(node->getValue());
    afterVisit();
}

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

void ASTPrinter::visitTypeAliasDecl(TypeAliasDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    afterVisit();
}

void ASTPrinter::visitVarDecl(VarDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    visit(node->getValue());
    afterVisit();
}

void ASTPrinter::visitVarLetDecl(VarLetDecl *node)
{
    beforeVisit(node);
    out << "\n";
    out << "Name: " << node->getName() << "Type: " << node->getType() << "\n";
    visit(node->getValue());
    afterVisit();
}

} // namespace glu::ast
