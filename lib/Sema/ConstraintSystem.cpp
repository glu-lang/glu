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
    , _diagManager(diagManager)
    , _context(context)
{
}

void ConstraintSystem::mapOverloadChoices(Solution *solution)
{
    for (auto &pair : solution->overloadChoices) {
        auto *refExpr = pair.first;
        auto *decl = pair.second;

        refExpr->setVariable(decl);
    }
}

void ConstraintSystem::mapImplicitConversions(Solution *solution)
{
    for (auto &pair : solution->implicitConversions) {
        auto *expr = pair.first;
        auto *targetType = pair.second;

        // Create a new CastExpr that wraps the original expression
        // auto *castExpr = _context->getASTMemoryArena().create<ast::CastExpr>(
        //     expr->getLocation(), expr, targetType
        // );

        // ast::replaceChild(expr, castExpr);
    }
}

bool ConstraintSystem::solveConstraints(
    llvm::ArrayRef<glu::ast::ExprBase *> expressions
)
{
    /// The initial system state used to begin constraint solving.
    std::vector<SystemState> worklist;
    worklist.emplace_back(); // Start from an empty state

    SolutionResult result; // Local solution result

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

        /// Apply defaultable constraints only if non-defaultable constraints
        /// succeed
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
            result.tryAddSolution(current);
    }

    if (result.isAmbiguous()) {
        _diagManager.error(
            SourceLocation::invalid,
            "Ambiguous type variable mapping found, cannot resolve."
        );
        return false;
    }

    Solution *solution = result.getBestSolution();

    if (!solution) {
        _diagManager.error(
            SourceLocation::invalid,
            "No best solution available for type variable mapping."
        );
        return false;
    }
    mapTypeVariables(solution);
    mapOverloadChoices(solution);
    mapImplicitConversions(solution);
    mapTypeVariablesToExpressions(solution, expressions);
    return true;
}

ConstraintResult
ConstraintSystem::applyBind(Constraint *constraint, SystemState &state)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    // Check if constraint is already satisfied
    auto *substitutedFirst = substitute(first, state.typeBindings, _context);
    auto *substitutedSecond = substitute(second, state.typeBindings, _context);
    if (substitutedFirst == substitutedSecond) {
        return ConstraintResult::Satisfied;
    }

    // Attempt to apply the constraint by unifying
    if (unify(first, second, state)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyDefaultable(Constraint *constraint, SystemState &state)
{
    auto *first = constraint->getFirstType();
    auto *second = constraint->getSecondType();

    // Check if constraint is already satisfied
    auto *substitutedFirst = substitute(first, state.typeBindings, _context);
    auto *substitutedSecond = substitute(second, state.typeBindings, _context);
    if (substitutedFirst == substitutedSecond) {
        return ConstraintResult::Satisfied;
    }

    auto *firstVar
        = llvm::dyn_cast<glu::types::TypeVariableTy>(substitutedFirst);
    if (!firstVar) {
        return ConstraintResult::Satisfied; // Not a type variable, nothing to
                                            // default
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
    auto *substitutedFirst = substitute(first, state.typeBindings, _context);
    auto *substitutedSecond = substitute(second, state.typeBindings, _context);

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
    fromType = substitute(fromType, state.typeBindings, _context);
    toType = substitute(toType, state.typeBindings, _context);

    // Check if already the same type (trivial conversion)
    if (fromType == toType) {
        return ConstraintResult::Satisfied;
    }

    // If either type is a type variable, attempt unification instead of
    // conversion checking
    if (llvm::isa<glu::types::TypeVariableTy>(fromType)
        || llvm::isa<glu::types::TypeVariableTy>(toType)) {
        if (unify(fromType, toType, state)) {
            return ConstraintResult::Applied;
        }
        return ConstraintResult::Failed;
    }

    // Use the conversion visitor for systematic conversion checking
    if (isValidConversion(fromType, toType, state, false)) {
        // Record the implicit conversion if the locator is an expression
        if (fromType == toType) {
            return ConstraintResult::Applied; // No conversion needed, recursive
                                              // unification happened
        }
        if (auto *expr
            = llvm::dyn_cast<glu::ast::ExprBase>(constraint->getLocator())) {
            state.implicitConversions[expr] = toType;
        }
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
    fromType = substitute(fromType, state.typeBindings, _context);
    toType = substitute(toType, state.typeBindings, _context);

    // Check if already the same type
    if (fromType == toType) {
        return ConstraintResult::Satisfied;
    }

    // Checked casts are more permissive than implicit conversions
    // Use the conversion visitor with explicit=true for checked casts
    if (isValidConversion(fromType, toType, state, true)) {
        return ConstraintResult::Applied;
    }

    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyBindOverload(Constraint *constraint, SystemState &state)
{
    auto *type = constraint->getOverload();
    auto *choice = constraint->getOverloadChoice();

    // Apply substitution to the type
    type = substitute(type, state.typeBindings, _context);

    // Get the function type from the chosen overload
    auto *functionType = choice->getType();

    // Check if already satisfied
    if (type == functionType) {
        return ConstraintResult::Satisfied;
    }

    // Try to unify the type with the function type
    if (unify(type, functionType, state)) {
        // Record the overload choice in the state
        if (auto *refExpr
            = llvm::dyn_cast<glu::ast::RefExpr>(constraint->getLocator())) {
            state.overloadChoices[refExpr] = choice;
            return ConstraintResult::Applied;
        }
    }

    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyLValueObject(Constraint *constraint, SystemState &state)
{
    auto *lvalueType = constraint->getFirstType();
    auto *objectType = constraint->getSecondType();

    // Apply substitutions
    lvalueType = substitute(lvalueType, state.typeBindings, _context);
    objectType = substitute(objectType, state.typeBindings, _context);

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
        return applyBindOverload(constraint, state);
    case ConstraintKind::LValueObject:
        return applyLValueObject(constraint, state);
    case ConstraintKind::Defaultable:
        return applyDefaultable(constraint, state);
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
    case ConstraintKind::ExpressibleByIntLiteral:
        return applyExpressibleByIntLiteral(constraint);
    case ConstraintKind::ExpressibleByFloatLiteral:
        return applyExpressibleByFloatLiteral(constraint);
    case ConstraintKind::ExpressibleByBoolLiteral:
        return applyExpressibleByBoolLiteral(constraint);
    case ConstraintKind::ExpressibleByStringLiteral:
        return applyExpressibleByStringLiteral(constraint);
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
    baseType = substitute(baseType, state.typeBindings, _context);
    memberType = substitute(memberType, state.typeBindings, _context);

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
    actualType = substitute(actualType, state.typeBindings, _context);
    expectedType = substitute(expectedType, state.typeBindings, _context);

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

    bool anySatisfied = false;

    for (auto *nestedConstraint : nestedConstraints) {
        // Try applying each nested constraint on a copy of the current state
        SystemState branchState = state.clone();

        ConstraintResult result
            = apply(nestedConstraint, branchState, worklist);

        switch (result) {
        case ConstraintResult::Satisfied:
            anySatisfied = true;
            // If a branch is satisfied, the current state already satisfies the
            // disjunction
            break;
        case ConstraintResult::Applied:
            // Add the successfully applied state to the worklist for further
            // exploration
            worklist.push_back(branchState);
            break;
        case ConstraintResult::Failed:
            // This branch failed, continue to next constraint
            break;
        }
    }

    // If any constraint was satisfied in the current state, the disjunction is
    // satisfied
    if (anySatisfied) {
        return ConstraintResult::Satisfied;
    }

    // If we have applied branches but no satisfied branches, we need to ensure
    // the current state gets updated properly. Since disjunctions represent
    // choice points, when we have multiple viable branches, we should pick one
    // for the current state to continue with.
    // Instead of continuing with an empty current state, fail this path
    // and rely on the branch states in the worklist. This ensures each
    // path through the constraint system represents a consistent choice.

    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyConjunction(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    // A conjunction succeeds only if all of its nested constraints succeed
    auto nestedConstraints = constraint->getNestedConstraints();

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
            break;
        case ConstraintResult::Failed:
            // If any constraint fails, the entire conjunction fails
            return ConstraintResult::Failed;
        }
    }

    // If any constraint was applied (and none failed), the conjunction is
    // applied
    if (anyApplied) {
        return ConstraintResult::Applied;
    }

    // All constraints were satisfied, but no modifications were made
    return ConstraintResult::Satisfied;
}

ConstraintResult
ConstraintSystem::applyExpressibleByIntLiteral(Constraint *constraint)
{
    auto exprTypeKind = constraint->getSingleType()->getKind();

    // Check if the expression type is an integer literal
    if (exprTypeKind == glu::types::TypeKind::IntTyKind) {
        // If it is, we can satisfy the constraint
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyExpressibleByFloatLiteral(Constraint *constraint)
{
    auto exprTypeKind = constraint->getSingleType()->getKind();

    // Check if the expression type is a float literal
    if (exprTypeKind == glu::types::TypeKind::FloatTyKind
        || exprTypeKind == glu::types::TypeKind::IntTyKind) {
        // If it is, we can satisfy the constraint
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyExpressibleByStringLiteral(Constraint *constraint)
{
    // Check if the expression type is a pointer
    if (auto exprType
        = llvm::dyn_cast<glu::types::PointerTy>(constraint->getSingleType())) {
        // Check if the expression type is a char pointer
        if (exprType->getPointee()->getKind()
            == glu::types::TypeKind::CharTyKind) {
            // If it is, we can satisfy the constraint
            return ConstraintResult::Satisfied;
        }
    }
    return ConstraintResult::Failed;
}

ConstraintResult
ConstraintSystem::applyExpressibleByBoolLiteral(Constraint *constraint)
{
    auto exprTypeKind = constraint->getSingleType()->getKind();

    // Check if the expression type is a boolean literal
    if (exprTypeKind == glu::types::TypeKind::BoolTyKind) {
        // If it is, we can satisfy the constraint
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

} // namespace glu::sema
