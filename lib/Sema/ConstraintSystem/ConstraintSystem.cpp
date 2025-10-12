#include "ConstraintSystem.hpp"

#include "AST/Expr/StructMemberExpr.hpp"
#include "AST/TypePrinter.hpp"
#include "AST/Types/EnumTy.hpp"
#include "AST/Types/StructTy.hpp"

#include <set>

namespace glu::sema {

ConstraintSystem::ConstraintSystem(
    ScopeTable *scopeTable, glu::DiagnosticManager &diagManager,
    glu::ast::ASTContext *context
)
    : _scopeTable(scopeTable)
    , _root(scopeTable->getNode())
    , _typeVariables()
    , _allocator()
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

static void insertImplicitCast(
    glu::ast::ASTContext *context, glu::ast::ExprBase *expr,
    types::TypeBase *targetType
)
{
    if (expr->getType() == targetType)
        return; // No conversion needed

    auto *parent = expr->getParent();
    // Create a new CastExpr that wraps the original expression
    auto *castExpr = context->getASTMemoryArena().create<ast::CastExpr>(
        expr->getLocation(), expr, targetType
    );
    castExpr->setType(targetType);

    ast::replaceChild(parent, expr, castExpr);
}

static bool tryCastFunctionCall(
    glu::ast::ASTContext *context, glu::ast::ExprBase *expr,
    types::TypeBase *targetType
)
{
    auto *refExpr = llvm::dyn_cast<ast::RefExpr>(expr);
    if (!refExpr)
        return false;
    auto *callExpr = llvm::dyn_cast<ast::CallExpr>(refExpr->getParent());
    if (!callExpr || callExpr->getCallee() != refExpr)
        return false;
    auto *functionTy = llvm::dyn_cast<types::FunctionTy>(refExpr->getType());
    auto *concreteTy = llvm::dyn_cast<types::FunctionTy>(targetType);
    if (!functionTy || !concreteTy)
        return false;
    // Handle return type
    insertImplicitCast(context, callExpr, concreteTy->getReturnType());
    // Handle parameters (don't touch variadic parameters)
    for (std::size_t i = 0;
         i < functionTy->getParameterCount() && i < callExpr->getArgs().size();
         ++i) {
        auto *paramTy = functionTy->getParameter(i);
        auto *argTy = concreteTy->getParameter(i);
        insertImplicitCast(context, callExpr->getArgs()[i], paramTy);
    }
    return true;
}

void ConstraintSystem::mapImplicitConversions(Solution *solution)
{
    for (auto &pair : solution->implicitConversions) {
        auto *expr = pair.first;
        auto *targetType = pair.second;

        if (!tryCastFunctionCall(_context, expr, targetType)) {
            insertImplicitCast(_context, expr, targetType);
        }
    }
}

bool ConstraintSystem::solveConstraints()
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
            if (constraint->getKind() == ConstraintKind::Defaultable
                || constraint->isTypePropertyConstraint()
                || constraint->getKind() == ConstraintKind::StructInitialiser) {
                continue;
            }

            /// Apply the constraint and check the result.
            ConstraintResult result = apply(constraint, current, worklist);
            markConstraint(result, constraint);
            if (result == ConstraintResult::Failed) {
                failed = true;
                break;
            }
            // Continue if Satisfied or Applied
        }

        if (failed)
            continue;

        for (Constraint *constraint : _constraints) {
            if (constraint->isDisabled())
                continue;
            if (constraint->getKind() != ConstraintKind::StructInitialiser)
                continue;
            /// Apply the constraint and check the result.
            ConstraintResult result = apply(constraint, current, worklist);
            markConstraint(result, constraint);
            if (result == ConstraintResult::Failed) {
                failed = true;
                break;
            }
        }

        /// Apply defaultable constraints only if non-defaultable
        /// constraints succeed
        for (Constraint *constraint : _constraints) {
            // Skip disabled constraints
            if (constraint->isDisabled())
                continue;

            // Only process defaultable constraints in second pass
            if (constraint->getKind() != ConstraintKind::Defaultable)
                continue;

            /// Apply the constraint and check the result.
            ConstraintResult result = apply(constraint, current, worklist);
            markConstraint(result, constraint);
            if (result == ConstraintResult::Failed) {
                failed = true;
                break;
            }
            // Continue if Satisfied or Applied
        }

        /// Apply ExpressibleByLiterals constraints only if other
        /// constraints succeed
        for (Constraint *constraint : _constraints) {
            // Skip disabled constraints
            if (constraint->isDisabled())
                continue;

            // Only process ExpressibleByLiterals constraints in third pass
            if (!constraint->isTypePropertyConstraint())
                continue;

            /// Apply the constraint and check the result.
            ConstraintResult result = apply(constraint, current, worklist);
            markConstraint(result, constraint);
            if (result == ConstraintResult::Failed) {
                failed = true;
                break;
            }
            // Continue if Satisfied or Applied
        }

        if (failed)
            continue;

        /// All constraints are satisfied -- record the solution.
        result.tryAddSolution(current);
    }

    if (result.isAmbiguous()) {
        reportAmbiguousSolutionError(result);
        return false;
    }

    Solution *solution = result.getBestSolution();

    if (!solution) {
        reportNoSolutionError();
        return false;
    }
    mapTypeVariables(solution);
    mapOverloadChoices(solution);
    mapImplicitConversions(solution);
    return true;
}

void ConstraintSystem::markConstraint(
    ConstraintResult result, Constraint *constraint
)
{
    if (result == ConstraintResult::Failed) {
        constraint->markFailed();
    } else {
        constraint->markSucceeded();
    }
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
        return ConstraintResult::Satisfied; // Not a type variable, nothing
                                            // to default
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

    // If second is already a pointer type, check if first matches its
    // element type
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
        // Substitute again
        fromType = substitute(fromType, state.typeBindings, _context);
        toType = substitute(toType, state.typeBindings, _context);
        // Record the implicit conversion if the locator is an expression
        if (fromType == toType) {
            return ConstraintResult::Applied; // No conversion needed,
                                              // recursive unification
                                              // happened
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
    case ConstraintKind::CheckedCast:
        return applyCheckedCast(constraint, state);
    case ConstraintKind::BindOverload:
        return applyBindOverload(constraint, state);
    case ConstraintKind::Defaultable:
        return applyDefaultable(constraint, state);
    // Complex constraint kinds that need special handling:
    case ConstraintKind::ValueMember:
        return applyValueMember(constraint, state);
    case ConstraintKind::Disjunction:
        return applyDisjunction(constraint, state, worklist);
    case ConstraintKind::Conjunction:
        return applyConjunction(constraint, state, worklist);
    case ConstraintKind::ExpressibleByIntLiteral:
        return applyExpressibleByIntLiteral(constraint, state);
    case ConstraintKind::ExpressibleByFloatLiteral:
        return applyExpressibleByFloatLiteral(constraint, state);
    case ConstraintKind::ExpressibleByBoolLiteral:
        return applyExpressibleByBoolLiteral(constraint, state);
    case ConstraintKind::ExpressibleByStringLiteral:
        return applyExpressibleByStringLiteral(constraint, state);
    case ConstraintKind::StructInitialiser:
        return applyStructInitialiser(constraint, state);
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
    auto *fieldType = field->getType();

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

ConstraintResult ConstraintSystem::applyDisjunction(
    Constraint *constraint, SystemState &state,
    std::vector<SystemState> &worklist
)
{
    // A disjunction succeeds if at least one of its nested constraints
    // succeeds
    auto nestedConstraints = constraint->getNestedConstraints();

    bool anySatisfied = false;

    for (auto *nestedConstraint : nestedConstraints) {
        // Try applying each nested constraint on a copy of the current
        // state
        SystemState branchState = state.clone();

        ConstraintResult result
            = apply(nestedConstraint, branchState, worklist);

        switch (result) {
        case ConstraintResult::Satisfied:
            anySatisfied = true;
            // If a branch is satisfied, the current state already satisfies
            // the disjunction
            break;
        case ConstraintResult::Applied:
            // Add the successfully applied state to the worklist for
            // further exploration
            worklist.push_back(branchState);
            break;
        case ConstraintResult::Failed:
            // This branch failed, continue to next constraint
            break;
        }
    }

    // If any constraint was satisfied in the current state, the disjunction
    // is satisfied
    if (anySatisfied) {
        return ConstraintResult::Satisfied;
    }

    // If we have applied branches but no satisfied branches, we need to
    // ensure the current state gets updated properly. Since disjunctions
    // represent choice points, when we have multiple viable branches, we
    // should pick one for the current state to continue with. Instead of
    // continuing with an empty current state, fail this path and rely on
    // the branch states in the worklist. This ensures each path through the
    // constraint system represents a consistent choice.

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

ConstraintResult ConstraintSystem::applyExpressibleByIntLiteral(
    Constraint *constraint, SystemState &state
)
{
    auto type
        = substitute(constraint->getSingleType(), state.typeBindings, _context);

    // Check if the expression type is an integer literal
    if (llvm::isa<glu::types::IntTy, glu::types::FloatTy>(type)) {
        // If it is, we can satisfy the constraint
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyExpressibleByFloatLiteral(
    Constraint *constraint, SystemState &state
)
{
    auto type
        = substitute(constraint->getSingleType(), state.typeBindings, _context);

    // Check if the expression type is a float literal
    if (llvm::isa<glu::types::FloatTy>(type)) {
        // If it is, we can satisfy the constraint
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyExpressibleByStringLiteral(
    Constraint *constraint, SystemState &state
)
{
    auto type
        = substitute(constraint->getSingleType(), state.typeBindings, _context);

    // Check if the expression type is a pointer
    if (auto exprType = llvm::dyn_cast<glu::types::PointerTy>(type)) {
        // Check if the expression type is a char pointer
        if (llvm::isa<glu::types::CharTy>(exprType->getPointee())) {
            // If it is, we can satisfy the constraint
            return ConstraintResult::Satisfied;
        }
    }
    if (auto exprType = llvm::dyn_cast<glu::types::StructTy>(type)) {
        // Check if the expression type is a String struct
        if (exprType->getName() == "String") {
            return ConstraintResult::Satisfied;
        }
    }
    // Check if the expression type is a character
    if (llvm::isa<glu::types::CharTy>(type)) {
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyExpressibleByBoolLiteral(
    Constraint *constraint, SystemState &state
)
{
    auto type
        = substitute(constraint->getSingleType(), state.typeBindings, _context);

    // Check if the expression type is a boolean literal
    if (llvm::isa<glu::types::BoolTy>(type)) {
        // If it is, we can satisfy the constraint
        return ConstraintResult::Satisfied;
    }
    return ConstraintResult::Failed;
}

ConstraintResult ConstraintSystem::applyStructInitialiser(
    Constraint *constraint, SystemState &state
)
{
    auto *type
        = substitute(constraint->getSingleType(), state.typeBindings, _context);
    auto *node
        = llvm::cast<ast::StructInitializerExpr>(constraint->getLocator());

    if (auto *structType = llvm::dyn_cast<glu::types::StructTy>(type)) {
        auto fields = structType->getFields();
        auto initialisers = node->getFields();
        for (size_t i = 0; i < fields.size() && i < initialisers.size(); i++) {
            auto fieldType = substitute(
                fields[i]->getType(), state.typeBindings, _context
            );
            if (!unify(fieldType, initialisers[i]->getType(), state)) {
                return ConstraintResult::Failed;
            }
        }
        return ConstraintResult::Applied;
    }
    return ConstraintResult::Failed;
}

void ConstraintSystem::reportAmbiguousSolutionError(
    SolutionResult const &result
)
{
    auto defaultLocation = _scopeTable->getNode()->getLocation();
    SourceLocation primaryLocation = defaultLocation;

    // Group overload choices by expression to show the different function
    // choices
    llvm::DenseMap<ast::ExprBase *, llvm::SmallVector<ast::FunctionDecl *, 2>>
        overloadsByExpr;

    for (auto const &solution : result.solutions) {
        for (auto const &[expr, decl] : solution.overloadChoices) {
            if (expr && decl) {
                overloadsByExpr[expr].push_back(decl);
                // Use the first expression's location as the primary location
                if (primaryLocation == defaultLocation) {
                    primaryLocation = expr->getLocation();
                }
            }
        }
    }

    // Now show the main error message
    _diagManager.error(
        primaryLocation,
        "Ambiguous type variable mapping found: multiple valid solutions "
        "exist; Consider adding explicit type annotations to resolve the "
        "ambiguity"
    );

    ast::TypePrinter printer;

    // Show the individual function notes first
    for (auto const &[expr, decls] : overloadsByExpr) {
        if (decls.size() <= 1)
            continue; // Not ambiguous if only one choice
        for (auto *decl : decls) {
            auto *funcType
                = llvm::dyn_cast<glu::types::FunctionTy>(decl->getType());
            if (!funcType)
                continue;

            _diagManager.note(
                decl->getLocation(),
                "Candidate of type: " + printer.visit(funcType)
            );
        }
    }
}

void ConstraintSystem::reportNoSolutionError()
{
    auto defaultLocation = _scopeTable->getNode()->getLocation();

    // Try to identify the most likely failing constraints and report them as
    // primary errors
    ast::TypePrinter printer;
    bool foundSpecificError = false;

    for (auto *constraint : _constraints) {
        if (constraint->isDisabled() || constraint->hasSucceeded()
            || !constraint->hasFailed())
            continue;

        auto *locator = constraint->getLocator();
        auto constraintLocation
            = locator ? locator->getLocation() : defaultLocation;

        switch (constraint->getKind()) {
        case ConstraintKind::Bind:
        case ConstraintKind::Equal: {
            auto *first = constraint->getFirstType();
            auto *second = constraint->getSecondType();

            // Provide more context about unification failures
            std::string firstDesc = getTypeDescription(first);
            std::string secondDesc = getTypeDescription(second);

            _diagManager.error(
                constraintLocation,
                "Type mismatch: expected " + firstDesc + ", found " + secondDesc
            );
            foundSpecificError = true;
            break;
        }
        case ConstraintKind::Conversion: {
            auto *fromType = constraint->getFirstType();
            auto *toType = constraint->getSecondType();

            std::string fromDesc = getTypeDescription(fromType);
            std::string toDesc = getTypeDescription(toType);
            std::string context
                = getConversionContext(constraint->getKind(), locator);

            _diagManager.error(
                constraintLocation,
                "Cannot convert " + fromDesc + " to " + toDesc + context
            );

            // If this is an overload resolution failure, show available
            // overloads
            if (auto *refExpr
                = llvm::dyn_cast_or_null<glu::ast::RefExpr>(locator)) {
                showAvailableOverloads(refExpr->getIdentifiers());
            }

            foundSpecificError = true;
            break;
        }
        case ConstraintKind::ValueMember: {
            auto *baseType = constraint->getFirstType();
            auto *memberType = constraint->getSecondType();

            std::string baseDesc = getTypeDescription(baseType);
            std::string memberDesc = getTypeDescription(memberType);

            _diagManager.error(
                constraintLocation,
                "Type " + baseDesc + " has no member of type " + memberDesc
            );
            foundSpecificError = true;
            break;
        }
        case ConstraintKind::ExpressibleByIntLiteral:
        case ConstraintKind::ExpressibleByFloatLiteral:
        case ConstraintKind::ExpressibleByBoolLiteral:
        case ConstraintKind::ExpressibleByStringLiteral: {
            auto *type = constraint->getSingleType();
            std::string typeDesc = getTypeDescription(type);
            std::string literalKind;

            switch (constraint->getKind()) {
            case ConstraintKind::ExpressibleByIntLiteral:
                literalKind = "integer literal";
                break;
            case ConstraintKind::ExpressibleByFloatLiteral:
                literalKind = "float literal";
                break;
            case ConstraintKind::ExpressibleByBoolLiteral:
                literalKind = "boolean literal";
                break;
            case ConstraintKind::ExpressibleByStringLiteral:
                literalKind = "string literal";
                break;
            default: literalKind = "literal"; break;
            }

            std::string message = "Cannot use " + literalKind;

            message += " as " + typeDesc;

            _diagManager.error(constraintLocation, message);
            foundSpecificError = true;
            break;
        }
        default:
            // For other constraint types, provide a generic message
            break;
        }
    }

    if (!foundSpecificError) {
        _diagManager.error(
            defaultLocation,
            "The type system could not infer types for this expression"
        );
        _diagManager.note(
            defaultLocation,
            "Try adding explicit type annotations to help the compiler"
        );
    }
}

std::string ConstraintSystem::getTypeDescription(glu::types::TypeBase *type)
{
    ast::TypePrinter printer;

    return printer.visit(type);
}

std::string ConstraintSystem::getConversionContext(
    ConstraintKind kind, glu::ast::ASTNode *locator
)
{
    switch (kind) {
    case ConstraintKind::Conversion:
        if (locator) {
            if (auto *assignStmt
                = llvm::dyn_cast<glu::ast::AssignStmt>(locator)) {
                return " in assignment";
            }
            if (auto *letDecl = llvm::dyn_cast<glu::ast::VarLetDecl>(locator)) {
                return " in initialization of variable '"
                    + letDecl->getName().str() + "'";
            }
            if (auto *retStmt = llvm::dyn_cast<glu::ast::ReturnStmt>(locator)) {
                return " in return statement";
            }
        }
        return "";
    default: return "";
    }
}

void ConstraintSystem::showAvailableOverloads(
    glu::ast::NamespaceIdentifier const &function
)
{
    // Search for all function declarations with the given name in the current
    // scope
    auto *scopeItem = _scopeTable->lookupItem(function);

    if (!scopeItem || scopeItem->decls.empty()) {
        return; // No functions found with this name
    }

    for (auto const &declWithVis : scopeItem->decls) {
        auto *decl = declWithVis.item;
        if (auto *funcDecl = llvm::dyn_cast<glu::ast::FunctionDecl>(decl)) {
            if (auto *funcType
                = llvm::dyn_cast<glu::types::FunctionTy>(funcDecl->getType())) {
                ast::TypePrinter printer;

                _diagManager.note(
                    funcDecl->getLocation(),
                    "Available overload: " + printer.visit(funcType)
                );
            }
        }
    }
}

} // namespace glu::sema
