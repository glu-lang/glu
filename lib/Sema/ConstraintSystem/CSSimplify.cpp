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

enum class ConstraintPriority : unsigned {
    // Priority 0: Immediate - simple deterministic bindings
    Immediate = 0,
    // Priority 1: Normal constraints
    Normal = 1,
    // Priority 2: Deferred constraints (like StructInitialiser)
    Deferred = 2,
    // Priority 3: Defaultable constraints (last resort)
    Defaultable = 3,
    // Priority 4: Type property constraints (checks only)
    TypeProperty = 4
};

static ConstraintPriority getPriority(Constraint *constraint)
{
    ConstraintKind kind = constraint->getKind();

    if (kind == ConstraintKind::Bind || kind == ConstraintKind::Equal
        || kind == ConstraintKind::BindToPointerType) {
        return ConstraintPriority::Immediate;
    }

    if (kind == ConstraintKind::StructInitialiser) {
        return ConstraintPriority::Deferred;
    }

    if (kind == ConstraintKind::Defaultable) {
        return ConstraintPriority::Defaultable;
    }

    if (constraint->isTypePropertyConstraint()) {
        return ConstraintPriority::TypeProperty;
    }

    return ConstraintPriority::Normal;
}

void ConstraintSystem::reorderConstraintsByPriority()
{
    // Reorder constraints by priority - lower priority number = processed first
    // This eliminates the need for multiple passes in solveLocalConstraints
    std::stable_sort(
        _constraints.begin(), _constraints.end(),
        [](Constraint *a, Constraint *b) {
            return getPriority(a) < getPriority(b);
        }
    );
}

} // namespace glu::sema
