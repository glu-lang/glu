#ifndef GLU_GILGEN_SCOPE_HPP
#define GLU_GILGEN_SCOPE_HPP

#include "Decls.hpp"
#include "Stmts.hpp"

#include "BasicBlock.hpp"

namespace glu::gilgen {

/// Represents a scope, in the GILGen sense. A scope is a compound statement in
/// the AST. This class is used to keep track of the current scope for GIL code
/// generation.
class Scope {
    /// The block statement this scope represents.
    ast::CompoundStmt *_block;
    /// The parent scope of this scope, or nullptr if this is the function
    /// scope.
    Scope *_parent;

    /// The basic block that represents the exit point of this scope, if this is
    /// a loop.
    gil::BasicBlock *_breakDestination = nullptr;
    /// The basic block that represents the continue point of this scope, if
    /// this is a loop.
    gil::BasicBlock *_continueDestination = nullptr;

    /// The variables declared in this scope.
    llvm::DenseMap<ast::VarLetDecl *, gil::Value> _variables;
    /// The allocations that must be cleaned up at the end of this scope.
    llvm::SmallVector<gil::Value, 4> _allocations;

public:
    /// @brief Creates a null scope.
    Scope(std::nullptr_t) : _block(nullptr), _parent(nullptr) { }

    /// @brief Creates a scope for a function.
    /// @param functionScope the AST function declaration.
    Scope(ast::FunctionDecl *functionScope)
        : _block(functionScope->getBody()), _parent(nullptr)
    {
    }

    /// @brief Creates a regular scope.
    /// @param functionScope the AST function declaration.
    Scope(ast::CompoundStmt *stmt, Scope *parent)
        : _block(stmt), _parent(parent)
    {
        assert(parent && "Parent scope must be provided");
    }

    /// Returns true if this scope represents a function.
    bool isFunctionScope() const
    {
        return llvm::isa<ast::FunctionDecl>(_block->getParent());
    }

    /// Returns true if this scope represents a loop (while or for).
    bool isLoopScope() const { return _breakDestination; }

    /// Sets the loop destination basic blocks for break and continue
    /// statements.
    void setLoopDestinations(
        gil::BasicBlock *breakDest, gil::BasicBlock *continueDest
    )
    {
        _breakDestination = breakDest;
        _continueDestination = continueDest;
    }

    /// Gets the parent scope, or nullptr if this is the function scope.
    Scope *getParent() const { return _parent; }

    /// Gets the break destination basic block for this scope.
    gil::BasicBlock *getBreakDestination() const { return _breakDestination; }

    /// Gets the continue destination basic block for this scope.
    gil::BasicBlock *getContinueDestination() const
    {
        return _continueDestination;
    }

    /// Inserts a variable into this scope's variable map.
    void insertVariable(ast::VarLetDecl *decl, gil::Value value)
    {
        _variables.insert({ decl, value });
        if (llvm::isa<ast::ParamDecl>(decl)
            && llvm::cast<ast::FunctionDecl>(decl->getParent())->getName()
                == "drop") {
            return; // Don't drop parameters in drop functions, otherwise
                    // infinite recursion
        }
        _allocations.push_back(value);
    }

    /// Adds an unnamed allocation to this scope.
    void addUnnamedAllocation(gil::Value value)
    {
        _allocations.push_back(value);
    }

    /// Gets the unnamed allocations for iteration purposes.
    llvm::SmallVector<gil::Value, 4> &getAllocations() { return _allocations; }

    /// Looks up a variable in this scope only (not parent scopes).
    std::optional<gil::Value> lookupVariableInScope(ast::VarLetDecl *decl) const
    {
        auto it = _variables.find(decl);
        if (it != _variables.end())
            return it->second;
        return std::nullopt;
    }

    /// Looks up a variable in this scope and all parent scopes.
    std::optional<gil::Value> lookupVariable(ast::VarLetDecl *decl) const
    {
        if (auto value = lookupVariableInScope(decl))
            return value;
        if (_parent)
            return _parent->lookupVariable(decl);
        return std::nullopt;
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_SCOPE_HPP
