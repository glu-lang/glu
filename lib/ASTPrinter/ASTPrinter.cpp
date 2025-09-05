#include "AST/TypePrinter.hpp"
#include "ASTWalker.hpp"
#include "Basic/SourceManager.hpp"
#include "Basic/Tokens.hpp"
#include <llvm/Support/WithColor.h>

namespace glu::ast {

inline llvm::raw_ostream &
operator<<(llvm::raw_ostream &out, glu::types::Case const &c)
{
    return out << c.name << " = " << c.value;
}

/// @brief Overloads the << operator to print the NodeKind as a string.
/// @param out The output stream to which the NodeKind will be printed.
/// @param kind The NodeKind enumeration value to be printed.
/// @return llvm::raw_ostream& The output stream after printing the NodeKind.
llvm::raw_ostream &operator<<(llvm::raw_ostream &out, NodeKind kind)
{
    switch (kind) {
#define NODE_KIND(Name, Parent)                     \
    case NodeKind::Name##Kind: return out << #Name;
#include "NodeKind.def"
    default: return out << "Unknown";
    }
}

static std::string printType(glu::types::TypeBase *type)
{
    if (type) {
        return ast::TypePrinter().visit(type);
    } else {
        return "nullptr";
    }
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &out, glu::ast::FieldDecl *c)
{
    return out << c->getName().str() << " = " << printType(c->getType())
               << "\n";
}

class ASTPrinter : public ASTVisitor<ASTPrinter> {
    SourceManager *_srcManager; ///< The source manager.
    llvm::raw_ostream &out; ///< The output stream to print the AST nodes.
    size_t _indent = 0; ///< The current indentation level.

public:
    /// @brief Called before visiting an AST node.
    /// @param node The AST node to be visited.
    void beforeVisitNode(ASTNode *node)
    {
        out.indent(_indent);

        // Print node kind with color
        llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << node->getKind();
        out << " " << node;

        bool isTopLevelOrDifferentFile = node->getParent() == nullptr
            || (_srcManager->getFileID(node->getParent()->getLocation()))
                != _srcManager->getFileID(node->getLocation());

        if (isTopLevelOrDifferentFile) {
            llvm::WithColor(out, llvm::raw_ostream::CYAN)
                << " <" << _srcManager->getBufferName(node->getLocation())
                << ", ";
        }

        if (!isTopLevelOrDifferentFile) {
            llvm::WithColor(out, llvm::raw_ostream::YELLOW) << " <";
        }

        llvm::WithColor(out, llvm::raw_ostream::YELLOW)
            << "line:"
            << _srcManager->getSpellingLineNumber(node->getLocation()) << ":"
            << _srcManager->getSpellingColumnNumber(node->getLocation()) << ">";

        if (auto *expr = llvm::dyn_cast<ExprBase>(node)) {
            llvm::WithColor(out, llvm::raw_ostream::GREEN)
                << " @type: " << printType(expr->getType());
        }

        out << "\n";

        _indent += 4;
    }

    /// @brief Called after visiting an AST node.
    /// @param node The AST node that was visited.
    void afterVisitNode([[maybe_unused]] ASTNode *node) { _indent -= 4; }

    /// @brief Constructs an ASTPrinter object.
    /// @param out The output stream to print the AST nodes. Defaults to
    /// llvm::outs().
    ASTPrinter(
        glu::SourceManager *srcManager, llvm::raw_ostream &out = llvm::outs()
    )
        : _srcManager(srcManager), out(out)
    {
    }

    void indent() { out.indent(_indent - 2); }

#define NODE_CHILD(Type, Name)                                      \
    node->get##Name()                                               \
        ? (out.indent(_indent - 2), out << "-->" << #Name << ":\n", \
           this->visit(node->get##Name()), (void) 0)                \
        : (void) 0
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)          \
    (void) 0;                              \
    out.indent(_indent - 2);               \
    out << "-->" << #Name << ":\n";        \
    for (auto child : node->get##Name()) { \
        this->visit(child);                \
    }                                      \
    (void) 0
#define NODE_KIND_(Name, Parent, ...)                             \
    void _visit##Name(Name *node)                                 \
    {                                                             \
        auto defer = llvm::make_scope_exit([&] { __VA_ARGS__; }); \
        return this->asImpl()->visit##Name(node);                 \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// STATEMENTS //////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits an AssignStmt node.
    /// @param node The AssignStmt node to be visited.
    void visitAssignStmt(AssignStmt *node)
    {
        out.indent(_indent - 4);
        llvm::WithColor(out, llvm::raw_ostream::BLUE)
            << "-->Operator: '" << node->getOperator() << "'\n";
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// DECLARATIONS ////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits a LetDecl node.
    /// @param node The LetDecl node to be visited.
    void visitLetDecl(LetDecl *node)
    {
        out.indent(_indent - 2);
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << "-->Name: " << node->getName() << "\n";
        out.indent(_indent - 2);
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << "-->Type: " << printType(node->getType()) << "\n";
    }

    /// @brief Visits a VarDecl node.
    /// @param node The VarDecl node to be visited.
    void visitVarDecl(VarDecl *node)
    {
        out.indent(_indent - 2);
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << "-->Name: " << node->getName() << '\n';
        out.indent(_indent - 2);
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << "-->Type: " << printType(node->getType()) << '\n';
    }
};

void ASTNode::debugPrint(llvm::raw_ostream &out)
{
    return ASTPrinter(getModule()->getSourceManager(), out)
        .visit(const_cast<ASTNode *>(this));
}

} // namespace glu::ast
