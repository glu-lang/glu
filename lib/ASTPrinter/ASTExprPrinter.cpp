#include "ASTPrinter.hpp"

void ASTPrinter::visitLiteralExpr(LiteralExpr *node)
{
    beforeVisit(node);
    afterVisit();
}
