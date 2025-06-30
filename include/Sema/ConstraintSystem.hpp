#ifndef GLU_SEMA_CONSTRAINT_SYSTEM_HPP
#define GLU_SEMA_CONSTRAINT_SYSTEM_HPP

#include "Constraint.hpp"
#include "ScopeTable.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/ilist.h>

namespace glu::sema {

/// @brief Represents a solution to a set of constraints.
struct Solution {
    /// @brief Inferred types for expressions (expr -> type).
    llvm::DenseMap<glu::ast::ExprBase *, glu::gil::Type *> exprTypes;

    /// @brief Type variable bindings (type variable -> type).
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::gil::Type *> typeBindings;

    /// @brief Overload choices made (expr -> function declaration).
    llvm::DenseMap<glu::ast::ExprBase *, glu::ast::FunctionDecl *>
        overloadChoices;

    /// @brief Retrieves the type of an expression (after resolution).
    /// @param expr The expression to query.
    /// @return The inferred type, or nullptr if not found.
    glu::gil::Type *getTypeFor(glu::ast::ExprBase *expr) const
    {
        auto it = exprTypes.find(expr);
        return (it != exprTypes.end()) ? it->second : nullptr;
    }

    /// @brief Records an inferred type for a given expression.
    /// @param expr The expression.
    /// @param type The inferred type.
    void recordExprType(glu::ast::ExprBase *expr, glu::gil::Type *type)
    {
        exprTypes[expr] = type;
    }

    /// @brief Binds a type variable to a specific type.
    /// @param var The type variable.
    /// @param type The type it is bound to.
    void bindTypeVar(glu::types::TypeVariableTy *var, glu::gil::Type *type)
    {
        typeBindings[var] = type;
    }

    /// @brief Records an overload resolution for a given expression.
    /// @param expr The expression.
    /// @param choice The selected function declaration.
    void
    recordOverload(glu::ast::ExprBase *expr, glu::ast::FunctionDecl *choice)
    {
        overloadChoices[expr] = choice;
    }
};

/// @brief Manages type constraints and their resolution in the current context.
class ConstraintSystem {
    ScopeTable *_scopeTable; ///< The scope table for the current context.
    std::vector<glu::types::TypeVariableTy *>
        _typeVariables; ///< List of type variables.
    llvm::BumpPtrAllocator _allocator; ///< Allocator for memory management.
    std::vector<Constraint *>
        _constraints; ///< List of constraints to be solved.
    llvm::DenseMap<Constraint *, std::pair<unsigned, Constraint *>>
        _bestSolutions; ///< Best solution per disjunction and its score.

public:
    /// @brief Constructs a ConstraintSystem.
    /// @param scopeTable The scope table for the current context.
    ConstraintSystem(ScopeTable *scopeTable);

    /// @brief Destroys the ConstraintSystem.
    ~ConstraintSystem() = default;

    /// @brief Gets the memory allocator.
    /// @return A reference to the allocator.
    llvm::BumpPtrAllocator &getAllocator() { return _allocator; }

    /// @brief Gets the scope table.
    /// @return The current scope table.
    ScopeTable *getScopeTable() { return _scopeTable; }

    /// @brief Gets the list of constraints.
    /// @return A reference to the vector of constraints.
    std::vector<Constraint *> &getConstraints() { return _constraints; }

    /// @brief Gets the best solution found for a given constraint.
    /// @param constraint The constraint for which to retrieve the solution.
    /// @return The best solution constraint, or nullptr if none.
    Constraint *getBestSolution(Constraint *constraint);

    /// @brief Gets the score of the best solution for a constraint.
    /// @param constraint The constraint to query.
    /// @return The score associated with the best solution.
    unsigned getBestSolutionScore(Constraint *constraint);

    /// @brief Gets the list of type variables.
    /// @return A reference to the list of type variables.
    std::vector<glu::types::TypeVariableTy *> &getTypeVariables()
    {
        return _typeVariables;
    }

    /// @brief Adds a new type variable to the system.
    /// @param typeVar The type variable to add.
    void addTypeVariable(glu::types::TypeVariableTy *typeVar)
    {
        _typeVariables.push_back(typeVar);
    }

    /// @brief Adds a new constraint to the system.
    /// @param constraint The constraint to add.
    void addConstraint(Constraint *constraint)
    {
        _constraints.push_back(constraint);
    }

    /// @brief Registers the best solution and score for a constraint.
    /// @param constraint The original constraint.
    /// @param solution The best constraint solution.
    /// @param score The associated score.
    void setBestSolution(
        Constraint *constraint, Constraint *solution, unsigned score
    )
    {
        _bestSolutions[constraint] = std::make_pair(score, solution);
    }

    /// @brief Solves all constraints added to the system.
    void solveConstraints()
    {
        // This function should implement the logic to resolve constraints
        // and find the best solutions based on the constraints added.
        // The implementation is not provided here, as it depends on the
        // specific requirements of the constraint resolution process.
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINT_SYSTEM_HPP
