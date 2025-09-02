#include "ConstraintSystem.hpp"

#include "AST/Expr/StructMemberExpr.hpp"
#include "AST/Types/StructTy.hpp"
#include "AST/TypePrinter.hpp"
#include "Basic/SourceManager.hpp"

namespace glu::sema {

ConstraintSystem::ConstraintSystem(
    ScopeTable *scopeTable, glu::DiagnosticManager &diagManager,
    glu::ast::ASTContext *context, bool enableLogging,
    llvm::raw_ostream &logStream, glu::SourceManager *sourceManager
)
    : _scopeTable(scopeTable)
    , _typeVariables()
    , _allocator()
    , _constraints()
    , _diagManager(diagManager)
    , _context(context)
    , _loggingEnabled(enableLogging)
    , _logStream(&logStream)
    , _logIndentLevel(0)
    , _typePrinter(enableLogging) // Enable type variable names when logging is enabled
    , _sourceManager(sourceManager)
{
    if (_loggingEnabled) {
        log("ConstraintSystem initialized with logging enabled");
    }
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

    if (_loggingEnabled) {
        log("====== Starting constraint solving ======");
        logAllConstraints();
        logSystemState(worklist[0], "Initial ");
        log("==========================================");
    }

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
    if (current.isFullyResolved(_constraints)) {
        if (_loggingEnabled) {
            log("Found complete solution:");
            increaseLogIndent();
            logSystemState(current, "Solution ");
            decreaseLogIndent();
        }
        result.tryAddSolution(current);
    }
}

if (result.isAmbiguous()) {
    if (_loggingEnabled) {
        log("RESULT: Ambiguous type variable mapping found");
    }
    _diagManager.error(
        SourceLocation::invalid,
        "Ambiguous type variable mapping found, cannot resolve."
    );
    return false;
}

Solution *solution = result.getBestSolution();

if (!solution) {
    if (_loggingEnabled) {
        log("RESULT: No best solution available");
    }
    _diagManager.error(
        SourceLocation::invalid,
        "No best solution available for type variable mapping."
    );
    return false;
}

if (_loggingEnabled) {
    log("RESULT: Successfully resolved constraints");
    log("====== Constraint solving completed ======");
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
    if (_loggingEnabled) {
        logConstraint(constraint, "Applying");
        increaseLogIndent();
        logSystemState(state, "Before: ");
    }
    
    ConstraintResult result;
    switch (constraint->getKind()) {
    case ConstraintKind::Bind: result = applyBind(constraint, state); break;
    case ConstraintKind::Equal: result = applyBind(constraint, state); break;
    case ConstraintKind::BindToPointerType:
        result = applyBindToPointerType(constraint, state); break;
    case ConstraintKind::Conversion: result = applyConversion(constraint, state); break;
    case ConstraintKind::ArgumentConversion:
        result = applyConversion(
            constraint, state
        ); break; // Same logic as regular conversion
    case ConstraintKind::OperatorArgumentConversion:
        result = applyConversion(
            constraint, state
        ); break; // Same logic as regular conversion
    case ConstraintKind::CheckedCast:
        result = applyCheckedCast(constraint, state); break;
    case ConstraintKind::BindOverload:
        result = applyBindOverload(constraint, state); break;
    case ConstraintKind::LValueObject:
        result = applyLValueObject(constraint, state); break;
    case ConstraintKind::Defaultable:
        result = applyDefaultable(constraint, state); break;
    // Complex constraint kinds that need special handling:
    case ConstraintKind::ValueMember:
        result = applyValueMember(constraint, state); break;
    case ConstraintKind::UnresolvedValueMember:
        result = applyUnresolvedValueMember(constraint, state); break;
    case ConstraintKind::GenericArguments:
        result = applyGenericArguments(constraint, state); break;
    case ConstraintKind::Disjunction:
        result = applyDisjunction(constraint, state, worklist); break;
    case ConstraintKind::Conjunction:
        result = applyConjunction(constraint, state, worklist); break;
    default: result = ConstraintResult::Failed; break;
    }
    
    if (_loggingEnabled) {
        logConstraintResult(result);
        logSystemState(state, "After: ");
        decreaseLogIndent();
    }
    
    return result;
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

// Logging implementations
void ConstraintSystem::log(llvm::StringRef message)
{
    if (!_loggingEnabled || !_logStream) {
        return;
    }
    
    *_logStream << getLogIndent() << "[ConstraintSystem] " << message << "\n";
    _logStream->flush();
}

void ConstraintSystem::logSystemState(SystemState const &state, llvm::StringRef prefix)
{
    if (!_loggingEnabled) {
        return;
    }
    
    log(prefix.str() + "System State:");
    increaseLogIndent();
    
    log("Type Bindings (" + std::to_string(state.typeBindings.size()) + "):");
    increaseLogIndent();
    for (auto const &binding : state.typeBindings) {
        std::string msg = formatType(binding.first) + 
                         " -> " + formatType(binding.second);
        log(msg);
    }
    decreaseLogIndent();
    
    log("Overload Choices (" + std::to_string(state.overloadChoices.size()) + "):");
    increaseLogIndent();
    for (auto const &choice : state.overloadChoices) {
        std::string msg = "RefExpr(" + std::to_string(reinterpret_cast<uintptr_t>(choice.first)) + 
                         ") -> " + formatFunctionDecl(choice.second);
        log(msg);
    }
    decreaseLogIndent();
    
    log("Implicit Conversions (" + std::to_string(state.implicitConversions.size()) + "):");
    increaseLogIndent();
    for (auto const &conversion : state.implicitConversions) {
        std::string msg = "Expr(" + std::to_string(reinterpret_cast<uintptr_t>(conversion.first)) + 
                         ") -> " + formatType(conversion.second);
        log(msg);
    }
    decreaseLogIndent();
    
    log("Score: " + std::to_string(state.score));
    
    decreaseLogIndent();
}

void ConstraintSystem::logConstraint(Constraint const *constraint, llvm::StringRef action)
{
    if (!_loggingEnabled || !constraint) {
        return;
    }
    
    std::string msg = action.str() + " constraint: ";
    switch (constraint->getKind()) {
    case ConstraintKind::Bind:
        msg += "Bind";
        break;
    case ConstraintKind::Equal:
        msg += "Equal";
        break;
    case ConstraintKind::Defaultable:
        msg += "Defaultable";
        break;
    case ConstraintKind::BindToPointerType:
        msg += "BindToPointerType";
        break;
    case ConstraintKind::Conversion:
        msg += "Conversion";
        break;
    case ConstraintKind::ArgumentConversion:
        msg += "ArgumentConversion";
        break;
    case ConstraintKind::OperatorArgumentConversion:
        msg += "OperatorArgumentConversion";
        break;
    case ConstraintKind::CheckedCast:
        msg += "CheckedCast";
        break;
    case ConstraintKind::BindOverload:
        msg += "BindOverload";
        break;
    case ConstraintKind::LValueObject:
        msg += "LValueObject";
        break;
    case ConstraintKind::ValueMember:
        msg += "ValueMember";
        break;
    case ConstraintKind::UnresolvedValueMember:
        msg += "UnresolvedValueMember";
        break;
    case ConstraintKind::GenericArguments:
        msg += "GenericArguments";
        break;
    case ConstraintKind::Disjunction:
        msg += "Disjunction";
        break;
    case ConstraintKind::Conjunction:
        msg += "Conjunction";
        break;
    }
    
    log(msg);
    
    // Log detailed constraint information
    std::string details = formatConstraintDetails(constraint);
    if (!details.empty()) {
        increaseLogIndent();
        log(details);
        decreaseLogIndent();
    }
}

void ConstraintSystem::logConstraintResult(ConstraintResult result)
{
    if (!_loggingEnabled) {
        return;
    }
    
    std::string msg = "Result: ";
    switch (result) {
    case ConstraintResult::Failed:
        msg += "Failed";
        break;
    case ConstraintResult::Satisfied:
        msg += "Satisfied";
        break;
    case ConstraintResult::Applied:
        msg += "Applied";
        break;
    }
    
    log(msg);
}

void ConstraintSystem::logAllConstraints()
{
    if (!_loggingEnabled) {
        return;
    }
    
    log("All Constraints (" + std::to_string(_constraints.size()) + "):");
    increaseLogIndent();
    for (size_t i = 0; i < _constraints.size(); ++i) {
        logConstraint(_constraints[i], "Constraint[" + std::to_string(i) + "]:");
    }
    decreaseLogIndent();
}

std::string ConstraintSystem::getLogIndent() const
{
    return std::string(_logIndentLevel * 2, ' ');
}

std::string ConstraintSystem::formatType(glu::types::Ty type) const
{
    if (!type) {
        return "<null>";
    }
    
    return _typePrinter.visit(type);
}

std::string ConstraintSystem::formatFunctionDecl(glu::ast::FunctionDecl *funcDecl) const
{
    if (!funcDecl) {
        return "<null>";
    }
    
    std::string result = funcDecl->getName().str();
    
    // Add type information
    if (auto funcType = funcDecl->getType()) {
        result += " : " + formatType(funcType);
    }
    
    // Add location information
    if (_sourceManager && funcDecl->getLocation().isValid()) {
        auto loc = funcDecl->getLocation();
        auto line = _sourceManager->getSpellingLineNumber(loc);
        auto column = _sourceManager->getSpellingColumnNumber(loc);
        auto fileName = _sourceManager->getBufferName(loc);
        
        // Extract just the filename without the full path
        auto lastSlash = fileName.find_last_of('/');
        if (lastSlash != llvm::StringRef::npos) {
            fileName = fileName.substr(lastSlash + 1);
        }
        
        result += " at " + fileName.str() + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
    
    return result;
}

std::string ConstraintSystem::formatConstraintDetails(Constraint const *constraint) const
{
    if (!constraint) {
        return "";
    }
    
    std::string result;
    
    switch (constraint->getKind()) {
    case ConstraintKind::Bind:
    case ConstraintKind::Equal:
    case ConstraintKind::Conversion:
    case ConstraintKind::ArgumentConversion:
    case ConstraintKind::OperatorArgumentConversion:
    case ConstraintKind::CheckedCast:
    case ConstraintKind::Defaultable:
    case ConstraintKind::LValueObject:
        result = formatType(constraint->getFirstType()) + " <-> " + formatType(constraint->getSecondType());
        break;
        
    case ConstraintKind::BindToPointerType:
        result = formatType(constraint->getFirstType()) + " (element) <-> *" + formatType(constraint->getSecondType());
        break;
        
    case ConstraintKind::BindOverload:
        result = formatType(constraint->getOverload()) + " -> " + formatFunctionDecl(constraint->getOverloadChoice());
        break;
        
    case ConstraintKind::ValueMember:
    case ConstraintKind::UnresolvedValueMember:
        result = formatType(constraint->getFirstType()) + " has member: " + formatType(constraint->getSecondType());
        if (auto member = constraint->getMember()) {
            result += " (member: " + member->getMemberName().str() + ")";
        }
        break;
        
    case ConstraintKind::GenericArguments:
        result = formatType(constraint->getFirstType()) + " with generic args: " + formatType(constraint->getSecondType());
        break;
        
    case ConstraintKind::Disjunction:
        result = "One of " + std::to_string(constraint->getNestedConstraints().size()) + " alternatives";
        break;
        
    case ConstraintKind::Conjunction:
        result = "All of " + std::to_string(constraint->getNestedConstraints().size()) + " conditions";
        break;
    }
    
    // Add restriction info if available
    if (constraint->hasRestriction()) {
        result += " [restriction: ";
        switch (constraint->getRestriction()) {
        case ConversionRestrictionKind::DeepEquality:
            result += "DeepEquality";
            break;
        case ConversionRestrictionKind::ArrayToPointer:
            result += "ArrayToPointer";
            break;
        case ConversionRestrictionKind::StringToPointer:
            result += "StringToPointer";
            break;
        case ConversionRestrictionKind::PointerToPointer:
            result += "PointerToPointer";
            break;
        }
        result += "]";
    }
    
    // Add flags
    std::vector<std::string> flags;
    if (constraint->isActive()) flags.push_back("active");
    if (constraint->isDisabled()) flags.push_back("disabled");
    if (constraint->isFavored()) flags.push_back("favored");
    if (constraint->isDiscarded()) flags.push_back("discarded");
    if (constraint->shouldRememberChoice()) flags.push_back("remember-choice");
    
    if (!flags.empty()) {
        result += " [";
        for (size_t i = 0; i < flags.size(); ++i) {
            if (i > 0) result += ", ";
            result += flags[i];
        }
        result += "]";
    }
    
    return result;
}

} // namespace glu::sema
