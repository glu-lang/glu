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

void ASTPrinter::visitFunctionDecl(FunctionDecl *node) { }

void ASTPrinter::visitLetDecl(LetDecl *node) { }

void ASTPrinter::visitStructDecl(StructDecl *node) { }

void ASTPrinter::visitTypeAliasDecl(TypeAliasDecl *node) { }

void ASTPrinter::visitVarDecl(VarDecl *node) { }

void ASTPrinter::visitVarLetDecl(VarLetDecl *node) { }

} // namespace glu::ast
