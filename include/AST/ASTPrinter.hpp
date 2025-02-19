#ifndef GLU_AST_PRINTER_HPP
#define GLU_AST_PRINTER_HPP

#include "ASTVisitor.hpp"

#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

/// @brief Overloads the << operator to print the NodeKind as a string.
/// @param out The output stream to which the NodeKind will be printed.
/// @param kind The NodeKind enumeration value to be printed.
/// @return llvm::raw_ostream& The output stream after printing the NodeKind.
llvm::raw_ostream &operator<<(llvm::raw_ostream &out, NodeKind kind)
{
    switch (kind) {
#define NODE_KIND(Name, Parent)                 \
case NodeKind::Name##Kind: return out << #Name;
#include "NodeKind.def"
    default: return out << "Unknown";
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
        out << node->getKind()
            << " at loc : " << node->getLocation().getOffset() << "\n";
    }
};

} // namespace glu::ast

#endif // GLU_AST_PRINTER_HPP
