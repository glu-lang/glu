#include "ConstraintSystem.hpp"

namespace glu::sema {

ConstraintSystem::ConstraintSystem(ScopeTable *scopeTable)
    : _scopeTable(scopeTable)
    , _typeVariables()
    , _allocator()
    , _constraints()
    , _bestSolutions()
{
}

Constraint *ConstraintSystem::getBestSolution(Constraint *constraint)
{
    auto it = _bestSolutions.find(constraint);
    if (it != _bestSolutions.end())
        return it->second.second;
    return nullptr;
}

unsigned ConstraintSystem::getBestSolutionScore(Constraint *constraint)
{
    auto it = _bestSolutions.find(constraint);
    if (it != _bestSolutions.end())
        return it->second.first;
    return 0;
}

void ConstraintSystem::solveConstraints()
{
    SolutionResult result;

    /// The initial system state used to begin constraint solving.
    std::vector<SystemState> worklist;
    worklist.emplace_back(); // Start from an empty state

    while (!worklist.empty()) {
        SystemState current = std::move(worklist.back());
        worklist.pop_back();

        bool failed = false;

        /// Apply each constraint to the current state.
        for (Constraint *constraint : _constraints) {
            /// If any constraint fails to apply, the current state is
            /// discarded.
            if (!apply(constraint, current, worklist)) {
                failed = true;
                break;
            }
        }

        if (failed)
            continue;

        /// If all constraints are satisfied and the state is
        /// complete, record it.
        if (current.isFullyResolved(_constraints))
            result.tryAddSolution(current);
    }

    // TODO: Use Result to update the best solutions for each constraint.
}

bool ConstraintSystem::apply(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    switch (constraint->getKind()) {
        // TODO: add different cases for each constraint kind
    }

    // Unknown constraint kind = failure
    return false;
}
}
