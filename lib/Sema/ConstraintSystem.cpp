#include "ConstraintSystem.hpp"
#include "AST/Expr/StructMemberExpr.hpp"
#include "AST/Types/StructTy.hpp"

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

bool ConstraintSystem::solveConstraints(
    llvm::ArrayRef<glu::ast::ExprBase *> expressions
)
{
    /// The initial system state used to begin constraint solving.
    std::vector<SystemState> worklist;
    worklist.emplace_back(); // Start from an empty state

    SolutionResult solutionResult; // Local solution result

    while (!worklist.empty()) {
        SystemState current = std::move(worklist.back());
        worklist.pop_back();

        bool failed = false;

        /// Apply non-defaultable constraints first
        for (Constraint *constraint : _constraints) {
            // Skip disabled constraints
            if (constraint->isDisabled())
                continue;

            // Skip defaultable constraints in first pass
            if (constraint->getKind() == ConstraintKind::Defaultable)
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

        /// Apply defaultable constraints only if non-defaultable constraints succeeded
        for (Constraint *constraint : _constraints) {
            // Skip disabled constraints
            if (constraint->isDisabled())
                continue;

            // Only process defaultable constraints in second pass
            if (constraint->getKind() != ConstraintKind::Defaultable)
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
            solutionResult.tryAddSolution(current);
    }

    // Apply type mappings to module expressions
    mapTypeVariables(solutionResult);

    // Get the best solution and apply it to the provided expressions
    Solution *solution = solutionResult.getBestSolution();
    if (!solution) {
        return false; // No solution found
    }

    // Update the best solutions for each constraint using the solution result
    updateBestSolutions(solutionResult);

    // Apply type mappings to the provided expressions
    mapTypeVariablesToExpressions(solution, expressions);
    return true;
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

    // Check if constraint is already satisfied
    auto *substitutedFirst = substitute(first, state.typeBindings);
    auto *substitutedSecond = substitute(second, state.typeBindings);
    if (substitutedFirst == substitutedSecond) {
        return ConstraintResult::Satisfied;
    }

    auto *firstVar
        = llvm::dyn_cast<glu::types::TypeVariableTy>(substitutedFirst);
    if (!firstVar) {
        return ConstraintResult::Satisfied; // Not a type variable, nothing to default
    }

    if (state.typeBindings.count(firstVar)) {
        return ConstraintResult::Satisfied; // Already bound, don't override
    }

    // Apply the default binding directly to the current state
    if (unify(first, second, state)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyBindToPointerType(
    Constraint *constraint, SystemState &state
)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    // Check if constraint is already satisfied
    auto *substitutedFirst = substitute(first, state.typeBindings);
    auto *substitutedSecond = substitute(second, state.typeBindings);

    // If second is already a pointer type, check if first matches its element
    // type
    if (auto *pointerType
        = llvm::dyn_cast<glu::types::PointerTy>(substitutedSecond)) {
        if (substitutedFirst == pointerType->getPointee()) {
            return ConstraintResult::Satisfied;
        }
        // Try to unify first with the pointee type
        if (unify(first, pointerType->getPointee(), state)) {
            return ConstraintResult::Applied;
        }
    }

    // If second is a type variable, bind it to a pointer type of first
    if (auto *secondVar
        = llvm::dyn_cast<glu::types::TypeVariableTy>(substitutedSecond)) {
        // Create pointer type: *first
        auto *pointerType
            = _context->getTypesMemoryArena().create<glu::types::PointerTy>(
                substitutedFirst
            );

        // Bind the type variable to the pointer type
        if (unify(second, pointerType, state)) {
            return ConstraintResult::Applied;
        }
    }

    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyConversion(Constraint *constraint, SystemState &state)
{
    auto *fromType = constraint->getFirstType();
    auto *toType = constraint->getSecondType();

    // Apply substitutions
    fromType = substitute(fromType, state.typeBindings);
    toType = substitute(toType, state.typeBindings);

    // Check if already the same type (trivial conversion)
    if (fromType == toType) {
        return ConstraintResult::Satisfied;
    }

    // FIXME: this should use a visitor pattern and be recursive
    // just like unify does.

    // If either type is a type variable, attempt unification instead of
    // conversion checking
    if (llvm::isa<glu::types::TypeVariableTy>(fromType)
        || llvm::isa<glu::types::TypeVariableTy>(toType)) {
        if (unify(fromType, toType, state)) {
            return ConstraintResult::Applied;
        }
        return ConstraintResult::Failed;
    }

    // Check if conversion is valid for concrete types
    if (isValidConversion(fromType, toType)) {
        // Record the implicit conversion if needed
        // Note: We might need to track which expression this applies to
        // For now, just accept the conversion
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyCheckedCast(Constraint *constraint, SystemState &state)
{
    auto *fromType = constraint->getFirstType();
    auto *toType = constraint->getSecondType();

    // Apply substitutions
    fromType = substitute(fromType, state.typeBindings);
    toType = substitute(toType, state.typeBindings);

    // Check if already the same type
    if (fromType == toType) {
        return ConstraintResult::Satisfied;
    }

    // Checked casts are more permissive than implicit conversions
    // They allow casts between pointers, integers, etc.
    if (isValidCheckedCast(fromType, toType)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyBindOverload(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    auto *type = constraint->getOverload();
    auto *choice = constraint->getOverloadChoice();

    // Apply substitution to the type
    type = substitute(type, state.typeBindings);

    // Get the function type from the chosen overload
    auto *functionType = choice->getType();

    // Check if already satisfied
    if (type == functionType) {
        return ConstraintResult::Satisfied;
    }

    // Try to unify the type with the function type
    if (unify(type, functionType, state)) {
        // Record the overload choice in the state
        SystemState newState = state;
        // Note: We need the expression this applies to, which isn't available
        // here This might need to be redesigned to pass the expression context
        worklist.push_back(newState);
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyLValueObject(Constraint *constraint, SystemState &state)
{
    auto *lvalueType = constraint->getFirstType();
    auto *objectType = constraint->getSecondType();

    // Apply substitutions
    lvalueType = substitute(lvalueType, state.typeBindings);
    objectType = substitute(objectType, state.typeBindings);

    // For now, just check if they're the same type
    // In a more complete implementation, we'd handle l-value semantics
    if (lvalueType == objectType) {
        return ConstraintResult::Satisfied;
    }

    // Try to unify them
    if (unify(lvalueType, objectType, state)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

bool ConstraintSystem::isValidConversion(
    glu::types::Ty fromType, glu::types::Ty toType
)
{
    // Same type is always valid
    if (fromType == toType) {
        return true;
    }

    // Integer conversions
    if (llvm::isa<glu::types::IntTy>(fromType)
        && llvm::isa<glu::types::IntTy>(toType)) {
        // Allow implicit conversions between integer types
        auto *fromInt = llvm::cast<glu::types::IntTy>(fromType);
        auto *toInt = llvm::cast<glu::types::IntTy>(toType);

        // Allow implicit widening conversions (smaller to larger)
        if (fromInt->getBitWidth() <= toInt->getBitWidth()) {
            return true;
        }
        // Narrowing conversions require explicit casts
        return false;
    }

    // Float conversions
    if (llvm::isa<glu::types::FloatTy>(fromType)
        && llvm::isa<glu::types::FloatTy>(toType)) {
        auto *fromFloat = llvm::cast<glu::types::FloatTy>(fromType);
        auto *toFloat = llvm::cast<glu::types::FloatTy>(toType);

        // Allow implicit widening of floats
        if (fromFloat->getBitWidth() <= toFloat->getBitWidth()) {
            return true;
        }
        return false;
    }

    // Array to pointer conversion
    if (auto *arrayType = llvm::dyn_cast<glu::types::StaticArrayTy>(fromType)) {
        if (auto *pointerType = llvm::dyn_cast<glu::types::PointerTy>(toType)) {
            return arrayType->getDataType() == pointerType->getPointee();
        }
    }

    // Other conversions would be added here...
    return false;
}

bool ConstraintSystem::isValidCheckedCast(
    glu::types::Ty fromType, glu::types::Ty toType
)
{
    // Checked casts are more permissive than implicit conversions
    if (isValidConversion(fromType, toType)) {
        return true;
    }

    // Allow casts between integers and pointers
    if ((llvm::isa<glu::types::IntTy>(fromType)
         && llvm::isa<glu::types::PointerTy>(toType))
        || (llvm::isa<glu::types::PointerTy>(fromType)
            && llvm::isa<glu::types::IntTy>(toType))) {
        return true;
    }

    // Allow casts between pointer types
    if (llvm::isa<glu::types::PointerTy>(fromType)
        && llvm::isa<glu::types::PointerTy>(toType)) {
        return true;
    }

    // Allow narrowing conversions for integers and floats in checked casts
    if (llvm::isa<glu::types::IntTy>(fromType)
        && llvm::isa<glu::types::IntTy>(toType)) {
        return true;
    }

    if (llvm::isa<glu::types::FloatTy>(fromType)
        && llvm::isa<glu::types::FloatTy>(toType)) {
        return true;
    }

    // Allow casts between enums and integers
    if ((llvm::isa<glu::types::EnumTy>(fromType)
         && llvm::isa<glu::types::IntTy>(toType))
        || (llvm::isa<glu::types::IntTy>(fromType)
            && llvm::isa<glu::types::EnumTy>(toType))) {
        return true;
    }

    return false;
}

ConstraintResult ConstraintSystem::apply(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    switch (constraint->getKind()) {
    case ConstraintKind::Bind: return applyBind(constraint, state);
    case ConstraintKind::Equal: return applyBind(constraint, state);
    case ConstraintKind::BindToPointerType:
        return applyBindToPointerType(constraint, state);
    case ConstraintKind::Conversion: return applyConversion(constraint, state);
    case ConstraintKind::ArgumentConversion:
        return applyConversion(
            constraint, state
        ); // Same logic as regular conversion
    case ConstraintKind::OperatorArgumentConversion:
        return applyConversion(
            constraint, state
        ); // Same logic as regular conversion
    case ConstraintKind::CheckedCast:
        return applyCheckedCast(constraint, state);
    case ConstraintKind::BindOverload:
        return applyBindOverload(constraint, state, worklist);
    case ConstraintKind::LValueObject:
        return applyLValueObject(constraint, state);
    case ConstraintKind::Defaultable:
        return applyDefaultable(constraint, state, worklist);
    // Complex constraint kinds that need special handling:
    case ConstraintKind::ValueMember:
        return applyValueMember(constraint, state);
    case ConstraintKind::UnresolvedValueMember:
        return applyUnresolvedValueMember(constraint, state);
    case ConstraintKind::GenericArguments:
        return applyGenericArguments(constraint, state);
    case ConstraintKind::Disjunction:
        return applyDisjunction(constraint, state, worklist);
    case ConstraintKind::Conjunction:
        return applyConjunction(constraint, state, worklist);
    default: return ConstraintResult::Failed;
    }
}

ConstraintResult
ConstraintSystem::applyValueMember(Constraint *constraint, SystemState &state)
{
    auto *baseType = constraint->getFirstType();
    auto *memberType = constraint->getSecondType();
    auto *memberExpr = constraint->getMember();

    // Apply substitutions
    baseType = substitute(baseType, state.typeBindings);
    memberType = substitute(memberType, state.typeBindings);

    // The base type should be a struct type
    auto *structType = llvm::dyn_cast<glu::types::StructTy>(baseType);
    if (!structType) {
        return ConstraintResult::Failed;
    }

    // Find the member in the struct
    llvm::StringRef memberName = memberExpr->getMemberName();
    auto fieldIndex = structType->getFieldIndex(memberName);
    if (!fieldIndex.has_value()) {
        return ConstraintResult::Failed; // Member not found
    }

    // Get the field type
    auto const &field = structType->getField(*fieldIndex);
    auto *fieldType = field.type;

    // Check if the member type matches the field type
    if (fieldType == memberType) {
        return ConstraintResult::Satisfied;
    }

    // Try to unify the member type with the field type
    if (unify(memberType, fieldType, state)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyUnresolvedValueMember(
    Constraint *constraint, SystemState &state
)
{
    // UnresolvedValueMember is similar to ValueMember, but the base type
    // might not be resolved yet. For now, treat it the same as ValueMember.
    // In a more complete implementation, this would handle cases where
    // the base type is inferred from the member access.
    return applyValueMember(constraint, state);
}

ConstraintResult ConstraintSystem::applyGenericArguments(
    Constraint *constraint, SystemState &state
)
{
    auto *actualType = constraint->getFirstType();
    auto *expectedType = constraint->getSecondType();

    // Apply substitutions
    actualType = substitute(actualType, state.typeBindings);
    expectedType = substitute(expectedType, state.typeBindings);

    // For now, implement a simple generic arguments constraint
    // that just unifies the actual and expected types
    if (actualType == expectedType) {
        return ConstraintResult::Satisfied;
    }

    // Try to unify the types
    if (unify(actualType, expectedType, state)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyDisjunction(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    // A disjunction succeeds if at least one of its nested constraints succeeds
    auto nestedConstraints = constraint->getNestedConstraints();

    bool anyApplied = false;
    bool anySatisfied = false;

    for (auto *nestedConstraint : nestedConstraints) {
        // Try applying each nested constraint on a copy of the current state
        SystemState branchState = state.clone();

        ConstraintResult result
            = apply(nestedConstraint, branchState, worklist);

        switch (result) {
        case ConstraintResult::Satisfied: anySatisfied = true; break;
        case ConstraintResult::Applied:
            // Add the successfully applied state to the worklist for further
            // exploration
            worklist.push_back(branchState);
            anyApplied = true;
            break;
        case ConstraintResult::Failed:
            // This branch failed, continue to next constraint
            break;
        }
    }

    // If any constraint was satisfied, the disjunction is satisfied
    if (anySatisfied) {
        return ConstraintResult::Satisfied;
    }

    // If any constraint was applied, the disjunction is applied
    if (anyApplied) {
        return ConstraintResult::Applied;
    }

    // All constraints failed
    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyConjunction(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    // A conjunction succeeds only if all of its nested constraints succeed
    auto nestedConstraints = constraint->getNestedConstraints();

    bool allSatisfied = true;
    bool anyApplied = false;

    // Apply all nested constraints to the current state
    for (auto *nestedConstraint : nestedConstraints) {
        ConstraintResult result = apply(nestedConstraint, state, worklist);

        switch (result) {
        case ConstraintResult::Satisfied:
            // This constraint is satisfied, continue
            break;
        case ConstraintResult::Applied:
            // This constraint was applied and modified the state
            anyApplied = true;
            allSatisfied = false;
            break;
        case ConstraintResult::Failed:
            // If any constraint fails, the entire conjunction fails
            return ConstraintResult::Failed;
        }
    }

    // If all constraints were satisfied, the conjunction is satisfied
    if (allSatisfied) {
        return ConstraintResult::Satisfied;
    }

    // If any constraint was applied (and none failed), the conjunction is
    // applied
    if (anyApplied) {
        return ConstraintResult::Applied;
    }

    // This shouldn't happen if the logic above is correct
    return ConstraintResult::Failed;
}

void ConstraintSystem::updateBestSolutions(SolutionResult &solutionResult)
{
    Solution *bestSolution = solutionResult.getBestSolution();
    if (!bestSolution) {
        return; // No solution to update from
    }

    Score bestScore = solutionResult.bestScore;

    // For each constraint, record the best solution found
    // This allows future queries via getBestSolution() to access the results
    for (Constraint *constraint : _constraints) {
        // Check if this constraint has been solved by examining relevant type variables
        bool constraintSolved = false;
        
        switch (constraint->getKind()) {
        case ConstraintKind::Bind:
        case ConstraintKind::Equal: {
            // For bind/equal constraints, check if the involved type variables have bindings
            auto *firstType = constraint->getFirstType();
            auto *secondType = constraint->getSecondType();
            
            // If either type is a type variable that got bound, consider constraint solved
            if (auto *firstVar = llvm::dyn_cast<glu::types::TypeVariableTy>(firstType)) {
                if (bestSolution->typeBindings.count(firstVar)) {
                    constraintSolved = true;
                }
            }
            if (auto *secondVar = llvm::dyn_cast<glu::types::TypeVariableTy>(secondType)) {
                if (bestSolution->typeBindings.count(secondVar)) {
                    constraintSolved = true;
                }
            }
            break;
        }
        case ConstraintKind::Defaultable: {
            // For defaultable constraints, check if the type variable got bound
            auto *firstType = constraint->getFirstType();
            if (auto *typeVar = llvm::dyn_cast<glu::types::TypeVariableTy>(firstType)) {
                if (bestSolution->typeBindings.count(typeVar)) {
                    constraintSolved = true;
                }
            }
            break;
        }
        case ConstraintKind::Conversion:
        case ConstraintKind::ArgumentConversion:
        case ConstraintKind::OperatorArgumentConversion:
        case ConstraintKind::CheckedCast:
        case ConstraintKind::BindToPointerType:
        case ConstraintKind::LValueObject:
        case ConstraintKind::ValueMember:
        case ConstraintKind::UnresolvedValueMember:
        case ConstraintKind::GenericArguments: {
            // For these constraint types, check type variable bindings
            auto *firstType = constraint->getFirstType();
            auto *secondType = constraint->getSecondType();
            
            if (auto *firstVar = llvm::dyn_cast<glu::types::TypeVariableTy>(firstType)) {
                if (bestSolution->typeBindings.count(firstVar)) {
                    constraintSolved = true;
                }
            }
            if (auto *secondVar = llvm::dyn_cast<glu::types::TypeVariableTy>(secondType)) {
                if (bestSolution->typeBindings.count(secondVar)) {
                    constraintSolved = true;
                }
            }
            break;
        }
        case ConstraintKind::BindOverload: {
            // For overload constraints, check if choice was recorded
            if (auto *expr = llvm::dyn_cast_or_null<glu::ast::RefExpr>(constraint->getLocator())) {
                if (bestSolution->overloadChoices.count(expr)) {
                    constraintSolved = true;
                }
            }
            break;
        }
        case ConstraintKind::Disjunction:
        case ConstraintKind::Conjunction:
            // For complex constraints, consider them solved if we reached a solution
            constraintSolved = true;
            break;
        }

        // If constraint was solved, record it in the best solutions cache
        if (constraintSolved) {
            // Store the constraint itself as the "solution" with the best score
            // This maintains the existing API expectations
            _bestSolutions[constraint] = std::make_pair(bestScore, constraint);
        }
    }
}

} // namespace glu::sema
