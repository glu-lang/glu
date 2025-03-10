#ifndef GLU_AST_PRINTER_HPP
#define GLU_AST_PRINTER_HPP

#include "ASTVisitor.hpp"
#include "Basic/SourceManager.hpp"
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
    glu::SourceManager *_srcManager; ///< The source manager.
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
    ASTPrinter(
        glu::SourceManager *srcManager, llvm::raw_ostream &out = llvm::outs()
    )
        : _srcManager(srcManager), out(out)
    {
    }
    void visitASTNode(ASTNode *node);
    void visitVarLetDecl(VarLetDecl *node);

#define NODE_KIND(Name, Parent) void visit##Name(Name *node);
#define NODE_KIND_SUPER(Name, Parent)
#define NODE_KIND_SUPER_END(Name)
#include "NodeKind.def"
};

inline llvm::raw_ostream &
operator<<(llvm::raw_ostream &out, glu::types::Field const &c)
{
    return out << c.name << " = " << c.type->getKind();
}

} // namespace glu::ast

#endif // GLU_AST_PRINTER_HPP
