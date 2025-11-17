#ifndef GLU_GILGEN_SCOPE_HPP
#define GLU_GILGEN_SCOPE_HPP

#include "Decls.hpp"
#include "Stmts.hpp"

#include "BasicBlock.hpp"

namespace glu::gilgen {

/// Represents a scope, in the GILGen sense. A scope is a compound statement in
/// the AST. This class is used to keep track of the current scope for GIL code
/// generation.
struct Scope {
    /// The block statement this scope represents.
    ast::CompoundStmt *block;
    /// The parent scope of this scope, or nullptr if this is the function
    /// scope.
    Scope *parent;

    /// The basic block that represents the exit point of this scope, if this is
    /// a loop.
    gil::BasicBlock *breakDestination = nullptr;
    /// The basic block that represents the continue point of this scope, if
    /// this is a loop.
    gil::BasicBlock *continueDestination = nullptr;

    /// The variables declared in this scope.
    llvm::DenseMap<ast::VarLetDecl *, gil::Value> variables;
    /// The "unnamed variables" (temporary allocations) in this scope.
    llvm::SmallVector<gil::Value, 4> unnamedAllocations;

public:
    /// @brief Creates a null scope.
    Scope(std::nullptr_t) : block(nullptr), parent(nullptr) { }

    /// @brief Creates a scope for a function.
    /// @param functionScope the AST function declaration.
    Scope(ast::FunctionDecl *functionScope)
        : block(functionScope->getBody()), parent(nullptr)
    {
    }

    /// @brief Creates a regular scope.
    /// @param functionScope the AST function declaration.
    Scope(ast::CompoundStmt *stmt, Scope *parent) : block(stmt), parent(parent)
    {
        assert(parent && "Parent scope must be provided");
    }

    /// Returns true if this scope represents a function.
    bool isFunctionScope() const
    {
        return llvm::isa<ast::FunctionDecl>(block->getParent());
    }

    /// Returns true if this scope represents a loop (while or for).
    bool isLoopScope() const { return breakDestination; }

    std::optional<gil::Value> lookupVariableInScope(ast::VarLetDecl *decl) const
    {
        auto it = variables.find(decl);
        if (it != variables.end())
            return it->second;
        return std::nullopt;
    }

    std::optional<gil::Value> lookupVariable(ast::VarLetDecl *decl) const
    {
        if (auto value = lookupVariableInScope(decl))
            return value;
        if (parent)
            return parent->lookupVariable(decl);
        return std::nullopt;
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_SCOPE_HPP
