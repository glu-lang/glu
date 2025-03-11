#include "ASTWalker.hpp"
#include "Basic/SourceManager.hpp"
#include "Basic/Tokens.hpp"

namespace glu::ast {

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
    switch (kind) {
#define TYPE(Name)                                              \
    case glu::types::TypeKind::Name##Kind: return out << #Name;
#include "Types/TypeKind.def"
    default: return out << "Unknown";
    }
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
    unsigned _printDetails = 0; ///< Whether to print details of the nodes.
    std::string _optMessage = ""; ///< Optional message to print.
    unsigned _optMessTiming = 0; ///< Timing to print the optional message.

public:
    /// @brief Called before visiting an AST node.
    void beforeVisitNode(ASTNode *node)
    {
        if (!_optMessage.empty() && _optMessTiming == 0) {
            out.indent(_indent - 2);
            out << _optMessage;
            _optMessage = "";
        }
        if (_printDetails == 0) {
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
                << "> ";
            _printDetails--;
            if (_optMessTiming > 0)
                _optMessTiming--;
        }
        _indent += 4;
    }

    /// @brief Called after visiting an AST node.
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

    /// @brief Visits an AST node.
    /// @param node The AST node to be visited.
    void visitASTNode(ASTNode *node)
    {
        if (!node)
            out << "Null ASTNode\n";
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// STATEMENTS
    /////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits an AssignStmt node.
    /// @param node The AssignStmt node to be visited.
    void visitAssignStmt(AssignStmt *node)
    {
        out.indent(_indent - 2);
        out << "-->" << node->getOperator() << " Assignement with:" << "\n";
        _printDetails = 2;
    }

    // /// @brief Visits an IfStmt node.
    // /// @param node The IfStmt node to be visited.
    // void visitIfStmt(IfStmt *node)
    // {
    //     printDetails(node);
    //     out << "If statement with condition:" << "\n";
    // }

    // /// @brief Visits a BreakStmt node.
    // /// @param node The BreakStmt node to be visited.
    // void visitBreakStmt(BreakStmt *node) { printDetails(node); }

    /// @brief Visits a CompoundStmt node.
    /// @param node The CompoundStmt node to be visited.
    // void visitCompoundStmt(CompoundStmt *node) { out << "\n"; }

    // /// @brief Visits a ContinueStmt node.
    // /// @param node The ContinueStmt node to be visited.
    // void visitContinueStmt(ContinueStmt *node) { out << "\n"; }

    /// @brief Visits an ExpressionStmt node.
    /// @param node The ExpressionStmt node to be visited.
    // void visitExpressionStmt(ExpressionStmt *node) { out << "\n"; }

    /// @brief Visits a ForStmt node.
    /// @param node The ForStmt node to be visited.
    void visitForStmt(ForStmt *node) { out << "\n"; }

    /// @brief Visits a ReturnStmt node.
    /// @param node The ReturnStmt node to be visited.
    void visitReturnStmt(ReturnStmt *node) { out << "\n"; }

    /// @brief Visits a WhileStmt node.
    /// @param node The WhileStmt node to be visited.
    void visitWhileStmt(WhileStmt *node) { out << "\n"; }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// DECLARATIONS
    ///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits an EnumDecl node.
    /// @param node The EnumDecl node to be visited.
    void visitEnumDecl(EnumDecl *node)
    {
        out.indent(_indent);
        out << "->" << "Name: " << node->getName() << "; Members : ";

        size_t caseCount = node->getType()->getCaseCount();
        for (size_t i = 0; i < caseCount; ++i) {
            out << node->getType()->getCase(i);
            if (i + 1 < caseCount) {
                out << ", ";
            }
        }
        out << "\n";
    }

    /// @brief Visits a FunctionDecl node.
    /// @param node The FunctionDecl node to be visited.
    void visitFunctionDecl(FunctionDecl *node) { out << "\n"; }

    /// @brief Visits a LetDecl node.
    /// @param node The LetDecl node to be visited.
    void visitLetDecl(LetDecl *node)
    {
        out.indent(_indent);
        out << "->" << "Name: " << node->getName()
            << "Type: " << node->getType() << "\n";
    }

    /// @brief Visits a StructDecl node.
    /// @param node The StructDecl node to be visited.
    void visitStructDecl(StructDecl *node)
    {
        out.indent(_indent);
        out << "Name: " << node->getName() << "; Fields : ";
        size_t fieldCount = node->getType()->getFieldCount();
        for (size_t i = 0; i < fieldCount; ++i) {
            out << node->getType()->getField(i);
            if (i + 1 < fieldCount) {
                out << ", ";
            }
        }
        out << "\n";
    }

    /// @brief Visits a TypeAliasDecl node.
    /// @param node The TypeAliasDecl node to be visited.
    void visitTypeAliasDecl(TypeAliasDecl *node)
    {
        out.indent(_indent);
        out << "->" << "Name: " << node->getName()
            << "Type: " << node->getType() << "\n";
    }

    /// @brief Visits a VarDecl node.
    /// @param node The VarDecl node to be visited.
    void visitVarDecl(VarDecl *node)
    {
        out.indent(_indent);
        out << "->" << "Name: " << node->getName()
            << "Type: " << node->getType() << "\n";
    }

    /// @brief Visits a VarLetDecl node.
    /// @param node The VarLetDecl node to be visited.
    void visitVarLetDecl(VarLetDecl *node)
    {
        out.indent(_indent);
        out << "->" << "Name: " << node->getName()
            << "Type: " << node->getType() << "\n";
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// EXPRESSIONS
    ////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Visits a LiteralExpr node.
    /// @param node The LiteralExpr node to be visited.
    void visitLiteralExpr(LiteralExpr *node)
    {
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
        out << "-->" << "Reference to: " << node->getIdentifier() << "\n";
    }

    void visitBinaryOpExpr(BinaryOpExpr *node)
    {
        out.indent(_indent - 2);
        out << "-->" << node->getOperator()
            << " Binary Operation with:" << "\n";
        _printDetails = 2;
    }

    void visitCallExpr(CallExpr *node)
    {
        out.indent(_indent - 2);
        out << "-->" << "Call to:\n";
        _printDetails = node->getArgs().size() + 1;
        _optMessage = "-->Arguments:\n";
        _optMessTiming = 1;
    }
};
void ASTNode::debugPrint(glu::SourceManager *srcManager, llvm::raw_ostream &out)
    const
{
    return ASTPrinter(srcManager, out).visit(const_cast<ASTNode *>(this));
}

} // namespace glu::ast
