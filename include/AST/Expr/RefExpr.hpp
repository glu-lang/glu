#ifndef GLU_AST_EXPR_REFEXPR_HPP
#define GLU_AST_EXPR_REFEXPR_HPP

#include "ASTNode.hpp"
#include "Decls.hpp"

#include <cstring>
#include <llvm/ADT/PointerUnion.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @struct NamespaceIdentifier
/// @brief Represents an identifier with namespaces maybe.
///
/// The following examples show how identifiers are decomposed:
///
/// - For "std::io::eprint":
///    - components: ["std", "io"]
///    - identifier: "eprint"
///
/// - For "llvm::APInt":
///    - components: ["llvm"]
///    - identifier: "APInt"
///
/// - For "machin":
///    - components: []
///    - identifier: "machin"
struct NamespaceIdentifier {
    llvm::ArrayRef<llvm::StringRef> components;
    llvm::StringRef identifier;

    std::string toString()
    {
        std::string result;

        for (auto &component : components) {
            result += component.str() + "::";
        }

        result += identifier.str();
        return result;
    }
};

/// @brief Represents a reference expression in the AST using trailing objects
/// to store the name string.
class RefExpr final : public ExprBase,
                      private llvm::TrailingObjects<RefExpr, llvm::StringRef> {
public:
    using ReferencedVarDecl = llvm::PointerUnion<VarLetDecl, FunctionDecl>;

private:
    using TrailingArgs = llvm::TrailingObjects<RefExpr, llvm::StringRef>;
    friend TrailingArgs;

    unsigned _numComponents;

    ReferencedVarDecl _variable;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(
        typename TrailingArgs::OverloadToken<llvm::StringRef>
    ) const
    {
        return _numComponents + 1; // +1 for the identifier
    }

    RefExpr(
        SourceLocation loc, NamespaceIdentifier const &identifier,
        ReferencedVarDecl variable
    )
        : ExprBase(NodeKind::RefExprKind, loc)
        , _numComponents(identifier.components.size())
        , _variable(variable)
    {
        std::uninitialized_copy(
            identifier.components.begin(), identifier.components.end(),
            getTrailingObjects<llvm::StringRef>()
        );
        std::memcpy(
            getTrailingObjects<llvm::StringRef>() + _numComponents,
            &identifier.identifier, sizeof(llvm::StringRef)
        );
    }

public:
    /// @brief Constructs a RefExpr object. (to be called via the memory arena)
    /// @param allocator The allocator of the memory arena (internal)
    /// @param loc the location of the reference expression
    /// @param name the name of the reference expression
    /// @param variable the variable declaration referenced by this reference
    /// @return the newly created RefExpr object
    static RefExpr *create(
        llvm::BumpPtrAllocator &allocator, SourceLocation loc,
        NamespaceIdentifier const &ident, ReferencedVarDecl variable = nullptr
    )
    {
        size_t totalSize
            = totalSizeToAlloc<llvm::StringRef>(ident.components.size() + 1);
        void *mem = allocator.Allocate(totalSize, alignof(RefExpr));
        return new (mem) RefExpr(loc, ident, variable);
    }

    /// @brief Get all identifiers of this reference expression.
    /// @return The identifier of this reference expression.
    NamespaceIdentifier getIdentifiers() const
    {
        llvm::StringRef const *trailing = getTrailingObjects<llvm::StringRef>();

        return NamespaceIdentifier {
            llvm::ArrayRef<llvm::StringRef>(trailing, _numComponents),
            trailing[_numComponents]
        };
    }

    /// @brief Get the identifier of this reference expression.
    /// @return The identifier of this reference expression.
    llvm::StringRef getIdentifier() const
    {
        return getTrailingObjects<llvm::StringRef>()[_numComponents];
    }

    /// @brief Get the variable declaration that this reference refers to.
    /// @return The variable declaration that this reference refers to.
    ReferencedVarDecl getVariable() const { return _variable; }

    /// @brief Set the variable declaration that this reference refers to.
    /// @param variable The variable declaration that this reference refers to.
    void setVariable(ReferencedVarDecl variable) { _variable = variable; }

    /// @brief Check if the node is a RefExpr.
    /// @param node The node to check.
    /// @return True if the node is a RefExpr, false otherwise.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::RefExprKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_EXPR_REFEXPR_HPP
