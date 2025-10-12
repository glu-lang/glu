#include "ConstraintSystem.hpp"

namespace glu::sema {

void SolutionResult::tryAddSolution(SystemState const &s)
{
    // If no previous solutions exist just add directly
    if (solutions.empty()) {
        solutions.push_back(std::move(s));
        bestScore = s.score;
        return;
    }

    Score newScore = s.score;

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

} // namespace glu::sema
