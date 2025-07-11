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

    mapTypeVariables(result);
    // TODO: Use Result to update the best solutions for each TypeVariableTy.
}

bool ConstraintSystem::applyBind(Constraint *constraint, SystemState &state)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    if (first == second)
        return true;

    if (auto *firstVar = llvm::dyn_cast<glu::types::TypeVariableTy>(first)) {
        auto existingBinding = state.typeBindings.find(firstVar);
        if (existingBinding != state.typeBindings.end()) {
            if (existingBinding->second != second)
                return false;
        }
        state.typeBindings[firstVar] = second;
        return true;
    } else if (auto *secondVar
               = llvm::dyn_cast<glu::types::TypeVariableTy>(second)) {
        auto existingBinding = state.typeBindings.find(secondVar);
        if (existingBinding != state.typeBindings.end()) {
            if (existingBinding->second != first)
                return false;
        }
    }
    return false;
}

bool ConstraintSystem::applyDefaultable(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    if (auto *firstVar = llvm::dyn_cast<glu::types::TypeVariableTy>(first)) {
        auto existingBinding = state.typeBindings.find(firstVar);
        if (existingBinding == state.typeBindings.end()) {
            SystemState appliedState = state;
            appliedState.typeBindings[firstVar] = second;
            worklist.push_back(appliedState);
            return true;
        } else if (existingBinding->second == second) {
            return true;
        } else {
            worklist.push_back(state);
            return true;
        }
    }
    return true;
}

bool ConstraintSystem::apply(
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
    default: return false;
    }
}

} // namespace glu::sema
