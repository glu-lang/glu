#ifndef GLU_AST_PRINTER_HPP
#define GLU_AST_PRINTER_HPP

#include "ASTVisitor.hpp"
#include "Types.hpp"

#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

/// @brief Overloads the << operator to print the NodeKind as a string.
/// @param out The output stream to which the NodeKind will be printed.
/// @param kind The NodeKind enumeration value to be printed.
/// @return llvm::raw_ostream& The output stream after printing the NodeKind.
inline llvm::raw_ostream &operator<<(llvm::raw_ostream &out, NodeKind kind)
{
    switch (kind) {
#define NODE_KIND(Name, Parent)                 \
case NodeKind::Name##Kind: return out << #Name;
#include "NodeKind.def"
    default: return out << "Unknown";
    }
}

/// @brief Overloads the << operator to print the NodeKind as a string.
/// @param out The output stream to which the NodeKind will be printed.
/// @param kind The NodeKind enumeration value to be printed.
/// @return llvm::raw_ostream& The output stream after printing the NodeKind.
inline llvm::raw_ostream &
operator<<(llvm::raw_ostream &out, glu::types::TypeKind kind)
{
    switch (kind) {
#define TYPE(Name)                                          \
case glu::types::TypeKind::Name##Kind: return out << #Name;
#include "Types/TypeKind.def"
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

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// Statements
    /////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// Declarations
    ///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    /// @brief Visits an EnumDecl node.
    /// @param node The EnumDecl node to be visited.
    void visitEnumDecl(EnumDecl *node);

    /// @brief Visits a FunctionDecl node.
    /// @param node The FunctionDecl node to be visited.
    void visitFunctionDecl(FunctionDecl *node);

    /// @brief Visits a LetDecl node.
    /// @param node The LetDecl node to be visited.
    void visitLetDecl(LetDecl *node);

    /// @brief Visits a StructDecl node.
    /// @param node The StructDecl node to be visited.
    void visitStructDecl(StructDecl *node);

    /// @brief Visits a TypeAliasDecl node.
    /// @param node The TypeAliasDecl node to be visited.
    void visitTypeAliasDecl(TypeAliasDecl *node);

    /// @brief Visits a VarDecl node.
    /// @param node The VarDecl node to be visited.
    void visitVarDecl(VarDecl *node);

    /// @brief Visits a VarLetDecl node.
    /// @param node The VarLetDecl node to be visited.
    void visitVarLetDecl(VarLetDecl *node);
};

inline llvm::raw_ostream &
operator<<(llvm::raw_ostream &out, glu::types::StructTy::Field const &c)
{
    return out << c.name << " = " << c.type->getKind();
}

} // namespace glu::ast

#endif // GLU_AST_PRINTER_HPP
