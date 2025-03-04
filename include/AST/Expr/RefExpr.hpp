#ifndef GLU_AST_EXPR_REFEXPR_HPP
#define GLU_AST_EXPR_REFEXPR_HPP

#include "ASTNode.hpp"
#include "Decls.hpp"

#include <cstring>
#include <llvm/ADT/PointerUnion.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @brief Represents a reference expression in the AST using trailing objects
/// to store the name string.
class RefExpr final : public ExprBase,
                      private llvm::TrailingObjects<RefExpr, char> {
public:
    using ReferencedVarDecl = llvm::PointerUnion<VarLetDecl, FunctionDecl>;

private:
    using TrailingArgs = llvm::TrailingObjects<RefExpr, char>;
    friend TrailingArgs;

    llvm::StringRef _name;
    size_t _nameLength;
    ReferencedVarDecl _variable;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<char>) const
    {
        return _nameLength;
    }

    RefExpr(
        SourceLocation loc, llvm::StringRef name, ReferencedVarDecl variable
    )
        : ExprBase(NodeKind::RefExprKind, loc)
        , _nameLength(name.size())
        , _variable(variable)
    {
        char *dest = getTrailingObjects<char>();
        std::memcpy(dest, name.data(), _nameLength);
        _name = llvm::StringRef(dest, _nameLength);
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
        llvm::StringRef name, ReferencedVarDecl variable = nullptr
    )
    {
        size_t totalSize = totalSizeToAlloc<char>(name.size());
        void *mem = allocator.Allocate(totalSize, alignof(RefExpr));
        return new (mem) RefExpr(loc, name, variable);
    }

    /// @brief Get the name of the reference.
    /// @return The name of the reference.
    llvm::StringRef getName() const { return _name; }

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
