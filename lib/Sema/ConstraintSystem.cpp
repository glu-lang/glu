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
        state.typeBindings[secondVar] = first;
        return true;
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
        }
    }
    return true;
}

glu::types::TypeBase *ConstraintSystem::resolveType(
    glu::types::TypeBase *type, SystemState const &state
) const
{
    if (auto *typeVar = llvm::dyn_cast<glu::types::TypeVariableTy>(type)) {
        auto it = state.typeBindings.find(typeVar);
        if (it != state.typeBindings.end()) {
            return it->second;
        }
    }
    return type;
}

bool ConstraintSystem::applyBindToPointerType(
    Constraint *constraint, SystemState &state
)
{
    auto *elementType = constraint->getFirstType();
    auto *pointerType = constraint->getSecondType();

    // Case 1: If the pointer type is a type variable, bind it to a pointer of
    // the element type
    if (auto *pointerVar
        = llvm::dyn_cast<glu::types::TypeVariableTy>(pointerType)) {
        // Resolve the element type
        auto *resolvedElementType = resolveType(elementType, state);

        // Check for existing binding first to avoid unnecessary allocation
        auto existingBinding = state.typeBindings.find(pointerVar);
        if (existingBinding != state.typeBindings.end()) {
            // Verify consistency with existing binding
            auto *existingType = existingBinding->second;
            if (auto *existingPtr
                = llvm::dyn_cast<glu::types::PointerTy>(existingType)) {
                // Check if the pointee types are compatible
                return existingPtr->getPointee() == resolvedElementType;
            }
            return false; // Incompatible existing binding
        }

        // Create a pointer type pointing to the resolved element type
        // (only if no existing binding)
        auto *newPointerType
            = _context->getTypesMemoryArena().create<glu::types::PointerTy>(
                resolvedElementType
            );

        // Bind the pointer type variable to the new pointer type
        state.typeBindings[pointerVar] = newPointerType;
        return true;
    }

    // Case 2: If the pointer type is already a concrete pointer type, verify
    // consistency
    if (auto *concretePtr
        = llvm::dyn_cast<glu::types::PointerTy>(pointerType)) {
        auto *concreteElementType = concretePtr->getPointee();

        // If element type is a type variable, bind it to the pointee type
        if (auto *elementVar
            = llvm::dyn_cast<glu::types::TypeVariableTy>(elementType)) {
            auto existingBinding = state.typeBindings.find(elementVar);
            if (existingBinding != state.typeBindings.end()) {
                return existingBinding->second == concreteElementType;
            }
            state.typeBindings[elementVar] = concreteElementType;
            return true;
        }

        // Both types are concrete - check for equality
        // Resolve element type for comparison
        auto *resolvedElementType = resolveType(elementType, state);
        return resolvedElementType == concreteElementType;
    }

    // Case 3: Neither type is a pointer type or type variable - this is an
    // error
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
    case ConstraintKind::Equal: return applyBind(constraint, state);
    case ConstraintKind::Defaultable:
        return applyDefaultable(constraint, state, worklist);
    case ConstraintKind::BindToPointerType:
        return applyBindToPointerType(constraint, state);
    // ...other constraint kinds not yet implemented...
    default: return false;
    }
}

} // namespace glu::sema
