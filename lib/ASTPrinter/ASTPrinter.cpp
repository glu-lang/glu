#include "AST/TypePrinter.hpp"
#include "ASTWalker.hpp"
#include "Basic/SourceManager.hpp"
#include "Basic/Tokens.hpp"
#include <llvm/Support/WithColor.h>

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

static std::string printType(glu::types::TypeBase *type)
{
    if (type) {
        return ast::TypePrinter().visit(type);
    } else {
        return "nullptr";
    }
}

class ASTPrinter : public ASTVisitor<ASTPrinter> {
    SourceManager *_srcManager; ///< The source manager.
    llvm::raw_ostream &out; ///< The output stream to print the AST nodes.
    size_t _indent = 0; ///< The current indentation level.

public:
    void printSourceLocation(ASTNode *node)
    {
        auto &sm = *_srcManager;
        bool isTopLevelOrDifferentFile = node->getParent() == nullptr
            || (sm.getFileID(node->getParent()->getLocation()))
                != sm.getFileID(node->getLocation());

        {
            llvm::WithColor yellow(out, llvm::raw_ostream::YELLOW);
            out << " <";
            if (node->getLocation().isInvalid()) {
                out << "invalid loc";
            } else {
                if (isTopLevelOrDifferentFile) {
                    out << sm.getBufferName(node->getLocation()) << ", ";
                }
                out << "line:" << sm.getSpellingLineNumber(node->getLocation())
                    << ":" << sm.getSpellingColumnNumber(node->getLocation());
            }
            out << ">";
        }
    }
    /// @brief Called before visiting an AST node.
    /// @param node The AST node to be visited.
    void beforeVisitNode(ASTNode *node)
    {
        out.indent(_indent);

        // Print node kind with color
        llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << node->getKind();
        out << " " << node;

        printSourceLocation(node);

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

    // - MARK: Statements

    /// @brief Visits an AssignStmt node.
    /// @param node The AssignStmt node to be visited.
    void visitAssignStmt(AssignStmt *node)
    {
        out.indent(_indent - 4);
        out << "-->Operator: '";
        llvm::WithColor(out, llvm::raw_ostream::BLUE)
            << node->getOperator() << "'\n";
    }

    // - MARK: Declarations

    void visitModuleDecl(ModuleDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Path: ";
        llvm::WithColor(out, llvm::raw_ostream::YELLOW)
            << node->getFilePath() << '\n';
        out.indent(_indent - 2);
        out << "-->Import Name: ";
        llvm::WithColor(out, llvm::raw_ostream::CYAN)
            << node->getImportName() << '\n';
    }

    /// @brief Visits an EnumDecl node.
    /// @param node The EnumDecl node to be visited.
    void visitEnumDecl(EnumDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->" << "Name: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << node->getName() << "\n";
    }

    /// @brief Visits a StructDecl node.
    /// @param node The StructDecl node to be visited.
    void visitStructDecl(StructDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << node->getName() << '\n';
        if (node->hasOverloadedDropFunction()) {
            out.indent(_indent - 2);
            out << "-->Drop function: " << node->getDropFunction();
            printSourceLocation(node->getDropFunction());
            out << '\n';
        }
        if (node->hasOverloadedCopyFunction()) {
            out.indent(_indent - 2);
            out << "-->Copy function: " << node->getCopyFunction();
            printSourceLocation(node->getCopyFunction());
            out << '\n';
        }
    }

    /// @brief Visits a TypeAliasDecl node.
    /// @param node The TypeAliasDecl node to be visited.
    void visitTypeAliasDecl(TypeAliasDecl *node)
    {
        out.indent(_indent - 2);
        out << "-->Name: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
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
        out.indent(_indent - 2);
        out << "-->Type: ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << printType(node->getType()) << '\n';
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

    // - MARK: Expressions

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
                } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    this->out << "Null";
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
        if (auto *varDecl
            = llvm::dyn_cast_if_present<VarLetDecl *>(node->getVariable())) {
            out << "-->" << "Reference to variable: ";
            llvm::WithColor(out, llvm::raw_ostream::CYAN)
                << varDecl->getName() << "\n";
        } else if (auto *funcDecl = llvm::dyn_cast_if_present<FunctionDecl *>(
                       node->getVariable()
                   )) {
            out << "-->" << "Reference to function: ";
            llvm::WithColor(out, llvm::raw_ostream::CYAN)
                << funcDecl->getName() << "\n";
        } else {
            out << "-->" << "Unresolved reference to: "
                << node->getIdentifiers().toString() << "\n";
        }
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

    // - MARK: Metadata

    void visitAttribute(Attribute *node)
    {
        out.indent(_indent - 2);
        out << "-->Kind: " << node->getAttributeKindName() << "\n";
    }
};

void ASTNode::print(llvm::raw_ostream &out)
{
    ASTPrinter(getModule()->getSourceManager(), out).visit(this);
}

// For use in LLDB:
// Those must be out-of-line, otherwise they're optimized away

void ASTNode::print()
{
    print(llvm::outs());
}

} // namespace glu::ast

void glu::types::TypeBase::print()
{
    llvm::outs() << glu::ast::TypePrinter().visit(this) << "\n";
}
