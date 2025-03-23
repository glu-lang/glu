#include "ASTWalker.hpp"
#include "Basic/SourceManager.hpp"
#include "Basic/Tokens.hpp"

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

/// @brief Overloads the << operator to print the NodeKind as a string.
/// @param out The output stream to which the NodeKind will be printed.
/// @param kind The NodeKind enumeration value to be printed.
/// @return llvm::raw_ostream& The output stream after printing the NodeKind.
llvm::raw_ostream &operator<<(llvm::raw_ostream &out, glu::types::TypeKind kind)
{
    llvm::StringRef kindStr;

    switch (kind) {
#define TYPE(Name)                                                 \
    case glu::types::TypeKind::Name##Kind: kindStr = #Name; break;
#include "Types/TypeKind.def"
    default: return out << "Unknown";
    }
    kindStr.consume_back("Ty");
    return out << kindStr;
}

llvm::raw_ostream &
operator<<(llvm::raw_ostream &out, glu::types::Field const &c)
{
    return out << c.name << " = " << c.type->getKind();
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &out, glu::Token const &token)
{
    switch (token.getKind()) {
#define TOKEN(Name)                                      \
    case glu::TokenKind::Name##Tok: return out << #Name;
#include "Basic/TokenKind.def"
    default: return out << "Unknown";
    }
}

class ASTPrinter : public ASTWalker<ASTPrinter, TraversalOrder::PreOrder> {
    SourceManager *_srcManager; ///< The source manager.
    llvm::raw_ostream &out; ///< The output stream to print the AST nodes.
    size_t _indent = 0; ///< The current indentation level.
    unsigned _printFileName = 0; ///< Whether to print details of the nodes.

public:
    /// @brief Called before visiting an AST node.
    /// @param node The AST node to be visited.
    void beforeVisitNode(ASTNode *node)
    {
        if (_printFileName == 0) {
            out.indent(_indent);
            out << node->getKind() << " " << node << " <"
                << _srcManager->getBufferName(node->getLocation()) << ", line:"
                << _srcManager->getSpellingLineNumber(node->getLocation())
                << ":"
                << _srcManager->getSpellingColumnNumber(node->getLocation())
                << ">\n";
        } else {
            out.indent(_indent);
            out << node->getKind() << " " << node << " <line:"
                << _srcManager->getSpellingLineNumber(node->getLocation())
                << ":"
                << _srcManager->getSpellingColumnNumber(node->getLocation())
                << ">\n";
            _printFileName--;
        }
        _indent += 4;
    }

    /// @brief Called after visiting an AST node.
    /// @param node The AST node that was visited.
    void afterVisitNode(ASTNode *node) { _indent -= 4; }

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

#define NODE_CHILD(Type, Name)                                            \
    node->get##Name()                                                     \
        ? (out.indent(_indent - 2), out << "-->" << #Name << ":\n",       \
           _printFileName += 1, this->visit(node->get##Name()), (void) 0) \
        : (void) 0
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)          \
    (void) 0;                              \
    out.indent(_indent - 2);               \
    out << "-->" << #Name << ":\n";        \
    for (auto child : node->get##Name()) { \
        _printFileName += 1;               \
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
        out << "-->" << node->getOperator() << " Assignement with:" << "\n";
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// DECLARATIONS ////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits an EnumDecl node.
    /// @param node The EnumDecl node to be visited.
    void visitEnumDecl(EnumDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->" << "Name: " << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Members:\n";

        size_t caseCount = node->getType()->getCaseCount();
        for (size_t i = 0; i < caseCount; ++i) {
            out.indent(_indent - 2);
            out << "|  " << node->getType()->getCase(i).name << " = "
                << node->getType()->getCase(i).value << "\n";
        }
    }

    /// @brief Visits a LetDecl node.
    /// @param node The LetDecl node to be visited.
    void visitLetDecl(LetDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: " << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Type: " << node->getType()->getKind() << "\n";
    }

    /// @brief Visits a StructDecl node.
    /// @param node The StructDecl node to be visited.
    void visitStructDecl(StructDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: " << node->getName() << '\n';
        out.indent(_indent - 2);
        out << "-->Fields:\n";
        size_t fieldCount = node->getType()->getFieldCount();
        for (size_t i = 0; i < fieldCount; ++i) {
            out.indent(_indent - 2);
            out << "|  " << node->getType()->getField(i).name << " : "
                << node->getType()->getField(i).type->getKind() << "\n";
        }
    }

    /// @brief Visits a TypeAliasDecl node.
    /// @param node The TypeAliasDecl node to be visited.
    void visitTypeAliasDecl(TypeAliasDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: " << node->getName() << "\n";
        out.indent(_indent - 2);
        out << "-->Type: " << node->getType()->getWrappedType()->getKind()
            << "\n";
    }

    /// @brief Visits a VarDecl node.
    /// @param node The VarDecl node to be visited.
    void visitVarDecl(VarDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: " << node->getName() << '\n';
        out.indent(_indent - 2);
        out << "-->Type: " << node->getType()->getKind() << '\n';
    }

    void visitParamDecl(ParamDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->" << node->getName() << " : " << node->getType()->getKind()
            << '\n';
    }

    void visitImportDecl(ImportDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Module: " << node->getImportPath().toString() << '\n';
    }

    void visitFunctionDecl(FunctionDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: " << node->getName() << '\n';
        out.indent(_indent - 2);
        out << "-->Return Type: " << node->getType()->getReturnType()->getKind()
            << '\n';
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// EXPRESSIONS /////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

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
                    this->out << "Integer: " << val;
                } else if constexpr (std::is_same_v<T, llvm::APFloat>) {
                    this->out << "Float: " << val.convertToDouble();
                } else if constexpr (std::is_same_v<T, llvm::StringRef>) {
                    this->out << "String: \"" << val.str() << "\"";
                } else if constexpr (std::is_same_v<T, bool>) {
                    this->out << "Boolean: " << (val ? "true" : "false");
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

    void visitBinaryOpExpr(BinaryOpExpr *node)
    {
        out.indent(_indent - 4);
        out << "-->" << node->getOperator()
            << " Binary Operation with:" << "\n";
        _printFileName += 2;
    }

    void visitCastExpr(CastExpr *node)
    {
        out.indent(_indent - 4);
        out << "-->Casting to " << node->getDestType()->getKind() << ":\n";
    }

    void visitStructMemberExpr(StructMemberExpr *node)
    {
        out.indent(_indent - 4);
        out << "-->Member: " << node->getMemberName() << " from struct:\n";
    }

    void visitUnaryOpExpr(UnaryOpExpr *node)
    {
        out.indent(_indent - 4);
        out << "-->" << node->getOperator() << " Unary Operation with:\n";
    }
};

void ASTNode::debugPrint(glu::SourceManager *srcManager, llvm::raw_ostream &out)
    const
{
    return ASTPrinter(srcManager, out).visit(const_cast<ASTNode *>(this));
}

} // namespace glu::ast
