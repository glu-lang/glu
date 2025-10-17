#include "ConstraintSystem.hpp"

#include "AST/Expr/CallExpr.hpp"
#include "AST/Expr/RefExpr.hpp"
#include "AST/Types/FunctionTy.hpp"
#include "AST/Types/TypeVariableTy.hpp"

#include <algorithm>

namespace glu::sema {

SystemState ConstraintSystem::simplifyConstraints()
{
    // Create initial state for simplification
    SystemState initialState(_context);

    reorderConstraintsByPriority();

    return initialState;
}

void ConstraintSystem::reorderConstraintsByPriority()
{
    // Reorder constraints by priority - lower priority number = processed first
    // This eliminates the need for multiple passes in solveLocalConstraints
    std::stable_sort(
        _constraints.begin(), _constraints.end(),
        [this](Constraint *a, Constraint *b) {
            unsigned priorityA = calculateConstraintPriority(a);
            unsigned priorityB = calculateConstraintPriority(b);
            return priorityA < priorityB;
        }
    );
}

unsigned ConstraintSystem::calculateConstraintPriority(Constraint *constraint)
{
    // Priority groups (lower = processed first)

    ConstraintKind kind = constraint->getKind();

    // Priority 0: Immediate - simple deterministic bindings
    if (kind == ConstraintKind::Bind || kind == ConstraintKind::Equal
        || kind == ConstraintKind::BindToPointerType) {
        return 0;
    }

    // (Priority 1: Everything else)

    // Priority 2: Struct initializers
    if (kind == ConstraintKind::StructInitialiser) {
        return 2;
    }

    // Priority 3: Defaultable
    if (kind == ConstraintKind::Defaultable) {
        return 3;
    }

    // Priority 4: Type property/literal constraints
    if (constraint->isTypePropertyConstraint()) {
        return 4;
    }

    // Priority 1: Everything else (Conversion, Disjunction, ValueMember, etc.)
    return 1;
}

} // namespace glu::sema
