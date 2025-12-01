#include "AST/Exprs.hpp"
#include "AST/Types.hpp"
#include "ASTVisitor.hpp"
#include "TypePrinter.hpp"

#include <llvm/Support/raw_ostream.h>
#include <string>

namespace glu::ast {

/// @brief CodePrinter is a visitor that converts AST nodes back to Glu source
/// code.
///
/// This class generates valid Glu source code from AST nodes, primarily for
/// decompilation of function prototypes and type declarations. It supports:
/// - FunctionDecl (without body)
/// - StructDecl
/// - EnumDecl
///
/// The generated code should be syntactically valid Glu code that can be
/// used as interface declarations.
class CodePrinter : public ASTVisitor<CodePrinter> {
    llvm::raw_ostream &_out; ///< The output stream to write the code to
    TypePrinter _typePrinter; ///< Type printer for formatting types
    size_t _indent = 0; ///< Current indentation level

public:
    /// @brief Constructs a CodePrinter object.
    /// @param out The output stream to write the generated code to.
    CodePrinter(llvm::raw_ostream &out = llvm::outs())
        : _out(out), _typePrinter(true /* enable type variable names */)
    {
    }

    /// @brief Visit a ModuleDecl and print its contents as Glu code
    /// @param node The ModuleDecl node to print
    void visitModuleDecl(ModuleDecl *node)
    {
        // Print each top-level declaration in the module
        for (auto *decl : node->getDecls()) {
            visit(decl);
            _out << "\n";
        }
    }

    /// @brief Visit a NamespaceDecl and print its contents as Glu code
    /// @param node The NamespaceDecl node to print
    void visitNamespaceDecl(NamespaceDecl *node)
    {
        printDeclPrefix(node);
        _out << "namespace " << escapeIdentifier(node->getName()) << " {\n";

        _indent += 4;
        for (auto *decl : node->getDecls()) {
            visit(decl);
            _out << "\n";
        }
        _indent -= 4;

        printIndent();
        _out << "}";
    }

    /// @brief Visit a FunctionDecl and print its signature (without body)
    /// @param node The FunctionDecl node to print
    void visitFunctionDecl(FunctionDecl *node)
    {
        printDeclPrefix(node);

        _out << "func " << escapeIdentifier(node->getName());

        printFunctionParameters(node->getParams());

        if (auto *funcType = node->getType()) {
            auto *returnType = funcType->getReturnType();
            if (!llvm::isa<glu::types::VoidTy>(returnType)) {
                _out << " -> ";
                printType(returnType);
            }
        }

        _out << ";";
    }

    /// @brief Visit a StructDecl and print its definition
    /// @param node The StructDecl node to print
    void visitStructDecl(StructDecl *node)
    {
        printDeclPrefix(node);

        _out << "struct " << escapeIdentifier(node->getName()) << " {\n";

        _indent += 4;

        for (auto *field : node->getFields()) {
            visit(field);
            _out << "\n";
        }

        _indent -= 4;

        printIndent();

        _out << "}";
    }

    /// @brief Visit an EnumDecl and print its definition
    /// @param node The EnumDecl node to print
    void visitEnumDecl(EnumDecl *node)
    {
        printDeclPrefix(node);

        _out << "enum " << escapeIdentifier(node->getName());
        if (auto *repr = node->getRepresentableType()) {
            _out << " : ";
            printType(repr);
        }
        _out << " {\n";

        _indent += 4;

        for (auto *field : node->getFields()) {
            visit(field);
            _out << "\n";
        }

        _indent -= 4;

        printIndent();

        _out << "}";
    }

    /// @brief Visit a FieldDecl and print its declaration
    /// @param node The FieldDecl node to print
    void visitFieldDecl(FieldDecl *node)
    {
        printDeclPrefix(node);

        _out << escapeIdentifier(node->getName());

        // For enum fields, we don't print the type (just the name)
        // For struct fields, we print "name: type"
        if (!llvm::isa_and_nonnull<EnumDecl>(node->getParent())) {
            _out << ": ";
            printType(node->getType());
        }

        _out << ",";
    }

    /// @brief Visit a ParamDecl and print its declaration
    /// @param node The ParamDecl node to print
    void visitParamDecl(ParamDecl *node)
    {
        visitAttributeList(node->getAttributes());
        _out << escapeIdentifier(node->getName()) << ": ";
        printType(node->getType());
    }

    /// @brief Visit a LiteralExpr and print its value for attribute parameters
    /// @param node The LiteralExpr node to print
    void visitLiteralExpr(LiteralExpr *node)
    {
        std::visit(
            [this](auto &&val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, llvm::APInt>) {
                    _out << val;
                } else if constexpr (std::is_same_v<T, llvm::APFloat>) {
                    _out << val.convertToDouble();
                } else if constexpr (std::is_same_v<T, llvm::StringRef>) {
                    _out << "\"" << val.str() << "\"";
                } else if constexpr (std::is_same_v<T, bool>) {
                    _out << (val ? "true" : "false");
                } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    _out << "null";
                }
            },
            node->getValue()
        );
    }

    void visitAttribute(Attribute *node)
    {
        _out << "@" << node->getAttributeKindSpelling();

        if (node->getParameter()) {
            _out << "(";
            visit(node->getParameter());
            _out << ")";
        }
    }

    void visitAttributeList(AttributeList *node)
    {
        if (!node)
            return;
        for (auto *attr : node->getAttributes()) {
            visit(attr);
            _out << " ";
        }
    }

private:
    /// @brief Print indentation
    void printIndent() { _out.indent(_indent); }

    /// @brief Print a type using the enhanced type printer
    /// @param type The type to print
    void printType(glu::types::TypeBase *type)
    {
        if (type) {
            _out << _typePrinter.visit(type);
        } else {
            _out << "void";
        }
    }

    /// @brief Print attributes and visibility prefix for declarations
    /// @param decl The declaration to print prefix for
    void printDeclPrefix(DeclBase *decl)
    {
        printIndent();
        visitAttributeList(decl->getAttributes());
        printVisibility(decl->getVisibility());
    }

    /// @brief Print function parameters
    /// @param params The parameter list
    void printFunctionParameters(llvm::ArrayRef<ParamDecl *> params)
    {
        _out << "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                _out << ", ";
            }
            visit(params[i]);
        }
        _out << ")";
    }

    /// @brief Print visibility modifier if present
    /// @param visibility The visibility to print
    void printVisibility(Visibility visibility)
    {
        switch (visibility) {
        case Visibility::Public: _out << "public "; break;
        case Visibility::Private: _out << "private "; break;
        }
    }
};

void ASTNode::printInterface(llvm::raw_ostream &out)
{
    CodePrinter(out).visit(this);
}

} // namespace glu::ast
