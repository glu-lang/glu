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

        llvm::WithColor(out, llvm::raw_ostream::YELLOW) << " <";

        if (isTopLevelOrDifferentFile) {
            llvm::WithColor(out, llvm::raw_ostream::YELLOW)
                << _srcManager->getBufferName(node->getLocation()) << ", ";
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
        out << "-->Operator: '";
        llvm::WithColor(out, llvm::raw_ostream::BLUE)
            << node->getOperator() << "'\n";
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// DECLARATIONS ////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits an EnumDecl node.
    /// @param node The EnumDecl node to be visited.
    void visitEnumDecl(EnumDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->" << "Name: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Members:\n";

        size_t caseCount = node->getType()->getCaseCount();
        for (size_t i = 0; i < caseCount; ++i) {
            out.indent(_indent - 2);
            out << "|  " << node->getType()->getCase(i).name << " = "
                << node->getType()->getCase(i).value << "\n";
        }
    }

    /// @brief Visits a StructDecl node.
    /// @param node The StructDecl node to be visited.
    void visitStructDecl(StructDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getName() << '\n';
    }

    /// @brief Visits a TypeAliasDecl node.
    /// @param node The TypeAliasDecl node to be visited.
    void visitTypeAliasDecl(TypeAliasDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Type: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()->getWrappedType()) << "\n";
    }

    void visitParamDecl(ParamDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->" << node->getName() << " : ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()) << '\n';
    }

    void visitFieldDecl(FieldDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Type: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()) << "\n";
    }

    void visitImportDecl(ImportDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Module: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getImportPath().toString() << '\n';
    }

    void visitFunctionDecl(FunctionDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getName() << '\n';
        // TODO: print parameters
        out.indent(_indent - 2);
        out << "-->Type: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()) << '\n';
    }
    /// @brief Visits a LiteralExpr node.
    /// @param node The LiteralExpr node to be visited.
    void visitLiteralExpr(LiteralExpr *node)
    {
        out.indent(_indent - 2);
        out << "-->";
        std::visit(
            [this](auto &&val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, llvm::APInt>) {
                    this->out << "Integer: ";
                    llvm::WithColor(out, llvm::raw_ostream::RED) << val;
                } else if constexpr (std::is_same_v<T, llvm::APFloat>) {
                    this->out << "Float: ";
                    llvm::WithColor(out, llvm::raw_ostream::RED)
                        << val.convertToDouble();
                } else if constexpr (std::is_same_v<T, llvm::StringRef>) {
                    this->out << "String: ";
                    llvm::WithColor(out, llvm::raw_ostream::RED)
                        << "\"" << val.str() << "\"";
                } else if constexpr (std::is_same_v<T, bool>) {
                    this->out << "Boolean: ";
                    llvm::WithColor(out, llvm::raw_ostream::RED)
                        << (val ? "true" : "false");
                } else {
                    this->out << "Unknown type";
                }
            },
            node->getValue()
        );
        out << "\n";
    }

    void visitRefExpr(RefExpr *node)
    {
        out.indent(_indent - 2);
        out << "-->" << "Reference to: " << node->getIdentifier() << "\n";
    }

    void visitCastExpr(CastExpr *node)
    {
        out.indent(_indent - 4);
        out << "-->Casting to " << printType(node->getDestType()) << ":\n";
    }

    void visitStructMemberExpr(StructMemberExpr *node)
    {
        out.indent(_indent - 4);
        out << "-->Member: " << node->getMemberName() << " from struct:\n";
    }

    /// @brief Visits a LetDecl node.
    /// @param node The LetDecl node to be visited.
    void visitLetDecl(LetDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Type: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()) << "\n";
    }

    /// @brief Visits a VarDecl node.
    /// @param node The VarDecl node to be visited.
    void visitVarDecl(VarDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getName() << '\n';
        out.indent(_indent - 2);
        out << "-->Type: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()) << '\n';
    }
};

void ASTNode::debugPrint(llvm::raw_ostream &out)
{
    return ASTPrinter(getModule()->getSourceManager(), out)
        .visit(const_cast<ASTNode *>(this));
}

} // namespace glu::ast
