#include "ConstraintSystem.hpp"

namespace glu::sema {

void SolutionResult::tryAddSolution(SystemState const &state)
{
    Solution s = state.toSolution();

    // If no previous solutions exist just add directly
    if (solutions.empty()) {
        solutions.push_back({ std::move(s), state.score });
        return;
    }

    unsigned newScore = state.score;
    unsigned bestScore = state.score; // Default fallback

    // Use the first solution as the reference for comparison
    for (auto const &sol : solutions) {
        // Compare the implicit conversion sizes to determine the best score
        unsigned currentScore = sol.score;
        if (currentScore < bestScore)
            bestScore = currentScore;
    }

    if (newScore < bestScore) {
        // if there is a better solution then replace previous ones
        solutions.clear();
        solutions.push_back({ std::move(s), state.score });
    } else if (newScore == bestScore) {
        // Ambiguity: multiple equally good solutions
        solutions.push_back({ std::move(s), state.score });
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
    for (Constraint *c : constraints)
        if (!c->isDisabled() && c->isActive())
            return false;
    return true;
}

} // namespace glu::sema
