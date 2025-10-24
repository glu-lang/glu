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
        : _out(out), _typePrinter(true, true) // Use readable type names
    {
    }

    /// @brief Visit a ModuleDecl and print its contents as Glu code
    /// @param node The ModuleDecl node to print
    void visitModuleDecl(ModuleDecl *node)
    {
        // Print each top-level declaration in the module
        for (auto *decl : node->getDecls()) {
            if (auto *funcDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                visit(funcDecl);
                _out << "\n";
            } else if (auto *structDecl = llvm::dyn_cast<StructDecl>(decl)) {
                visit(structDecl);
                _out << "\n";
            } else if (auto *enumDecl = llvm::dyn_cast<EnumDecl>(decl)) {
                visit(enumDecl);
                _out << "\n";
            }
            // Skip other declaration types as they're not supported
        }
    }

    /// @brief Visit a FunctionDecl and print its signature (without body)
    /// @param node The FunctionDecl node to print
    void visitFunctionDecl(FunctionDecl *node)
    {
        printIndent();

        printVisibility(node->getVisibility());

        _out << "func " << node->getName();

        printFunctionParameters(node->getParams());

        if (auto *funcType
            = llvm::dyn_cast<glu::types::FunctionTy>(node->getType())) {
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
        printIndent();

        printVisibility(node->getVisibility());

        _out << "struct " << node->getName() << " {\n";

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
        printIndent();

        printVisibility(node->getVisibility());

        _out << "enum " << node->getName() << " {\n";

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
        printIndent();
        _out << node->getName() << ": ";
        printType(node->getType());

        bool isFieldInStructOrEnum
            = llvm::isa_and_nonnull<StructDecl>(node->getParent())
            || llvm::isa_and_nonnull<EnumDecl>(node->getParent());

        _out << (isFieldInStructOrEnum ? "," : ";");
    }

    /// @brief Visit a ParamDecl and print its declaration
    /// @param node The ParamDecl node to print
    void visitParamDecl(ParamDecl *node)
    {
        _out << node->getName() << ": ";
        printType(node->getType());
    }

    /// @brief Default handler for nodes that shouldn't be printed
    /// @param node The AST node
    void beforeVisitNode([[maybe_unused]] ASTNode *node)
    {
        // Do nothing by default - override specific visit methods
    }

    /// @brief Default handler for nodes that shouldn't be printed
    /// @param node The AST node
    void afterVisitNode([[maybe_unused]] ASTNode *node)
    {
        // Do nothing by default - override specific visit methods
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
        case Visibility::Private:
            // Private is the default, don't print anything
            break;
        }
    }
};

void ASTNode::printCode(llvm::raw_ostream &out)
{
    CodePrinter(out).visit(this);
}

} // namespace glu::ast
