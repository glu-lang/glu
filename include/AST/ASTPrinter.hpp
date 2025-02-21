#ifndef GLU_AST_PRINTER_HPP
#define GLU_AST_PRINTER_HPP

#include "ASTVisitor.hpp"
#include "Stmts.hpp"

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

/// @class ASTPrinter
/// @brief A class that prints the Abstract Syntax Tree (AST) nodes.
///
/// This class is derived from ASTVisitor and is used to print the AST nodes
/// to the specified output stream.
class ASTPrinter : public ASTVisitor<ASTPrinter> {
    llvm::raw_ostream &out; ///< The output stream to print the AST nodes.
    size_t _indent = 0; ///< The current indentation level.

    /// @brief Called before visiting an AST node.
    void beforeVisit(ASTNode *node);

    /// @brief Called after visiting an AST node.
    void afterVisit();

public:
    /// @brief Constructs an ASTPrinter object.
    /// @param out The output stream to print the AST nodes. Defaults to
    /// llvm::outs().
    ASTPrinter(llvm::raw_ostream &out = llvm::outs()) : out(out) { }

    /// @brief Visits an AST node.
    /// @param node The AST node to be visited.
    void visitASTNode(ASTNode *node);

    /// @brief Visits a BreakStmt node.
    /// @param node The BreakStmt node to be visited.
    void visitBreakStmt(BreakStmt *node);

    /// @brief Visits a CompoundStmt node.
    /// @param node The CompoundStmt node to be visited.
    void visitCompoundStmt(CompoundStmt *node);

    /// @brief Visits a ContinueStmt node.
    /// @param node The ContinueStmt node to be visited.
    void visitContinueStmt(ContinueStmt *node);

    /// @brief Visits an ExpressionStmt node.
    /// @param node The ExpressionStmt node to be visited.
    void visitExpressionStmt(ExpressionStmt *node);

    /// @brief Visits an IfStmt node.
    /// @param node The IfStmt node to be visited.
    void visitIfStmt(IfStmt *node);

    /// @brief Visits a ReturnStmt node.
    /// @param node The ReturnStmt node to be visited.
    void visitReturnStmt(ReturnStmt *node);

    /// @brief Visits a WhileStmt node.
    /// @param node The WhileStmt node to be visited.
    void visitWhileStmt(WhileStmt *node);
};

} // namespace glu::ast

#endif // GLU_AST_PRINTER_HPP
