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

void ConstraintSystem::mapOverloadChoices(Solution *solution)
{
    for (auto &pair : solution->overloadChoices) {
        auto *refExpr = pair.first;
        auto *decl = pair.second;

        refExpr->setVariable(decl);
    }
}

void ConstraintSystem::mapExprTypes(Solution *solution)
{
    for (auto &pair : solution->exprTypes) {
        auto *expr = pair.first;
        auto *type = pair.second;

        expr->setType(type);
    }
}

void ConstraintSystem::mapImplicitConversions(Solution *solution)
{
    for (auto &pair : solution->implicitConversions) {
        auto *expr = pair.first;
        auto *targetType = pair.second;
        auto *castExpr = _context->getASTMemoryArena().create<ast::CastExpr>(
            expr->getLocation(), expr, targetType
        );

        castExpr->setParent(expr->getParent());
        expr->setParent(castExpr);
    }
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

    if (result.isAmbiguous()) {
        _diagManager.error(
            SourceLocation::invalid,
            "Ambiguous type variable mapping found, cannot resolve."
        );
        return;
    }

    Solution *solution = result.getBestSolution();

    if (!solution) {
        _diagManager.error(
            SourceLocation::invalid,
            "No best solution available for type variable mapping."
        );
        return;
    }
    mapTypeVariables(solution);
    mapOverloadChoices(solution);
    mapExprTypes(solution);
    mapImplicitConversions(solution);
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

bool ConstraintSystem::apply(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    switch (constraint->getKind()) {
        // TODO: add different cases for each constraint kind
    case ConstraintKind::Bind: return applyBind(constraint, state);
    // ...other constraint kinds not yet implemented...
    default: return false;
    }
}

} // namespace glu::sema
