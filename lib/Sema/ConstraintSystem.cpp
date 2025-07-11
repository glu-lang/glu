#include "ConstraintSystem.hpp"

namespace glu::sema {

ConstraintSystem::ConstraintSystem(
    ScopeTable *scopeTable, glu::DiagnosticManager &diagManager,
    glu::ast::ASTContext *context
)
    : _scopeTable(scopeTable)
    , _typeVariables()
    , _allocator()
    , _constraints()
    , _bestSolutions()
    , _diagManager(diagManager)
    , _context(context)
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
            // Skip disabled constraints
            if (constraint->isDisabled())
                continue;

            /// Apply the constraint and check the result.
            ConstraintResult result = apply(constraint, current, worklist);
            if (result == ConstraintResult::Failed) {
                failed = true;
                break;
            }
            // Continue if Satisfied or Applied
        }

        if (failed)
            continue;

        /// If all constraints are satisfied, record the solution.
        if (current.isFullyResolved(_constraints))
            result.tryAddSolution(current);
    }

    mapTypeVariables(result);
    // TODO: Use Result to update the best solutions for each TypeVariableTy.
}

ConstraintResult
ConstraintSystem::applyBind(Constraint *constraint, SystemState &state)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    // Check if constraint is already satisfied
    auto *substitutedFirst = substitute(first, state.typeBindings);
    auto *substitutedSecond = substitute(second, state.typeBindings);
    if (substitutedFirst == substitutedSecond) {
        return ConstraintResult::Satisfied;
    }

    // Attempt to apply the constraint by unifying
    if (unify(first, second, state)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyDefaultable(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    auto *firstVar = llvm::dyn_cast<glu::types::TypeVariableTy>(first);
    if (!firstVar || state.typeBindings.count(firstVar)) {
        return ConstraintResult::Satisfied; // Already bound or not a type
                                            // variable
    }

    // Create new state with the default binding
    SystemState appliedState = state;
    appliedState.typeBindings[firstVar] = second;
    worklist.push_back(appliedState);
    return ConstraintResult::Applied;
}

ConstraintResult ConstraintSystem::apply(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    switch (constraint->getKind()) {
        // TODO: add different cases for each constraint kind
    case ConstraintKind::Bind: return applyBind(constraint, state);
    case ConstraintKind::Equal: return applyBind(constraint, state);
    case ConstraintKind::Defaultable:
        return applyDefaultable(constraint, state, worklist);
    // ...other constraint kinds not yet implemented...
    default: return ConstraintResult::Failed;
    }
}

} // namespace glu::sema
