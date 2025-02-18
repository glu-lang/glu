#ifndef GLU_AST_PRINTER_HPP
#define GLU_AST_PRINTER_HPP

#include "ASTVisitor.hpp"
#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

class ASTPrinter : public ASTVisitor<ASTPrinter, void> {
    llvm::raw_ostream &out;

public:
    ASTPrinter(llvm::raw_ostream &out = llvm::outs()) : out(out) { }

    void visitASTNode(ASTNode *node) { out << "Unknown ASTNode\n"; }

#define NODE_KIND(Name, Parent)                               \
    void visit##Name(ASTNode *node) { out << #Name << "\n"; }

#define NODE_KIND_SUPER(Name, Parent) NODE_KIND(Name, Parent)
#include "NodeKind.def"

    void print(ASTNode *node) { visit(node); }
};

} // namespace glu::ast

#endif // GLU_AST_PRINTER_HPP
