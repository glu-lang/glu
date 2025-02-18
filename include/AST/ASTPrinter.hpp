#ifndef GLU_AST_PRINTER_HPP
#define GLU_AST_PRINTER_HPP

#include "ASTVisitor.hpp"
#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

inline char const *toString(NodeKind kind)
{
    switch (kind) {
#define NODE_KIND(Name, Parent)          \
case NodeKind::Name##Kind: return #Name;
#include "NodeKind.def"
    default: return "Unknown";
    }
}

class ASTPrinter : public ASTVisitor<ASTPrinter, void> {
    llvm::raw_ostream &out;

public:
    ASTPrinter(llvm::raw_ostream &out = llvm::outs()) : out(out) { }

    void visitASTNode(ASTNode *node)
    {
        if (!node) {
            out << "Null ASTNode\n";
            return;
        }
        out << toString(node->getKind())
            << " at loc : " << node->getLocation().getOffset() << "\n";
    }
};

} // namespace glu::ast

#endif // GLU_AST_PRINTER_HPP
