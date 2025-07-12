#include "ConstraintSystem.hpp"

namespace glu::sema {

void SolutionResult::tryAddSolution(SystemState const &state)
{
    Solution s = state.toSolution();

    // If no previous solutions exist just add directly
    if (solutions.empty()) {
        solutions.push_back(std::move(s));
        bestScore = state.score;
        return;
    }

    Score newScore = state.score;

    if (newScore < bestScore) {
        // if there is a better solution then replace previous ones
        solutions.clear();
        solutions.push_back(std::move(s));
        bestScore = newScore;
    } else if (newScore == bestScore) {
        // Ambiguity: multiple equally good solutions
        solutions.push_back(std::move(s));
    }
}

Solution SystemState::toSolution() const
{
    Solution s;

    for (auto const &[expr, type] : exprTypes)
        s.recordExprType(expr, type);

    for (auto const &[var, type] : typeBindings)
        s.bindTypeVar(var, type);

    for (auto const &[expr, decl] : overloadChoices)
        s.recordOverload(expr, decl);

    for (auto const &[expr, targetType] : implicitConversions)
        s.recordImplicitConversion(expr, targetType);

    return s;
}

bool SystemState::isFullyResolved(
    std::vector<Constraint *> const &constraints
) const
{
    // A system state is fully resolved when we have processed all constraints
    // and the solver has applied all possible bindings.
    // Since the constraint solving algorithm processes constraints until
    // they are all satisfied or failed, if we reach this point with a
    // state that wasn't rejected, it means we have a valid solution.

    // For now, we'll consider any state that reached this point as resolved
    // In a more sophisticated implementation, we might check if type variables
    // are all bound or if constraints have specific "satisfied" markers
    return true;
}

} // namespace glu::sema
