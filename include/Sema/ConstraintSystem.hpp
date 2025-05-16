#ifndef GLU_SEMA_CONSTRAINT_SYSTEM_HPP
#define GLU_SEMA_CONSTRAINT_SYSTEM_HPP

#include "Constraint.hpp"
#include "ScopeTable.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/ilist.h>

namespace glu::sema {

class ConstraintSystem {
    ScopeTable *_scopeTable; // The scope table for the current context
    std::vector<glu::types::TypeVariableTy *>
        _typeVariables; // List of type variables
    llvm::BumpPtrAllocator _allocator; // Allocator for memory management
    std::vector<Constraint *> _constraints; // List of constraints
    llvm::DenseMap<Constraint *, std::pair<unsigned, Constraint *>>
        _bestSolutions; // Best solution for Disjunctions and their scores
public:
    /// @brief Constructor for ConstraintSystem.
    /// @param scopeTable The scope table for the current context.
    ConstraintSystem(ScopeTable *scopeTable);

    /// @brief Destructor for ConstraintSystem.
    ~ConstraintSystem() = default;

    /// @brief Returns the allocator used for memory management.
    llvm::BumpPtrAllocator &getAllocator() { return _allocator; }

    /// @brief Returns the scope table for the current context.
    ScopeTable *getScopeTable() { return _scopeTable; }

    /// @brief Returns the list of constraints.
    std::vector<Constraint *> &getConstraints() { return _constraints; }

    Constraint *getBestSolution(Constraint *constraint);

    unsigned getBestSolutionScore(Constraint *constraint);

    /// @brief Returns the list of type variables.
    std::vector<glu::types::TypeVariableTy *> &getTypeVariables()
    {
        return _typeVariables;
    }

    /// @brief Adds a type variable to the list.
    /// @param typeVar The type variable to add.
    void addTypeVariable(glu::types::TypeVariableTy *typeVar)
    {
        _typeVariables.push_back(typeVar);
    }

    /// @brief Creates a new constraint and adds it to the list.
    void addConstraint(Constraint *constraint)
    {
        _constraints.push_back(constraint);
    }

    void setBestSolution(
        Constraint *constraint, Constraint *solution, unsigned score
    )
    {
        _bestSolutions[constraint] = std::make_pair(score, solution);
    }
};

}

#endif // GLU_SEMA_CONSTRAINT_SYSTEM_HPP
