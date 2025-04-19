#include "Constraints.hpp"

namespace glu::sema {

Constraint::Constraint(
    ConstraintKind kind, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : _kind(kind)
    , _numTypeVariables(typeVars.size())
    , _hasFix(false)
    , _hasRestriction(false)
    , _isActive(false)
    , _isDisabled(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _nested(constraints)
    , _locator(locator)
{
    assert(
        kind == ConstraintKind::Disjunction
        || kind == ConstraintKind::Conjunction
    );
    std::uninitialized_copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : _kind(kind)
    , _numTypeVariables(typeVars.size())
    , _hasFix(false)
    , _hasRestriction(false)
    , _isActive(false)
    , _isDisabled(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _types { first, second }
    , _locator(locator)
{
    assert(first && "First type is Null");
    assert(second && "Second type is Null");

    switch (_kind) {
    case ConstraintKind::Bind:
    case ConstraintKind::Equal:
    case ConstraintKind::BindToPointerType:
    case ConstraintKind::Conversion:
    case ConstraintKind::ArgumentConversion:
    case ConstraintKind::OperatorArgumentConversion:
    case ConstraintKind::CheckedCast:
    case ConstraintKind::ExplicitGenericArguments:
    case ConstraintKind::LValueObject: break;
    case ConstraintKind::ValueMember:
    case ConstraintKind::UnresolvedValueMember:
    case ConstraintKind::Defaultable:

    case ConstraintKind::BindOverload:
        llvm_unreachable("Wrong constructor for overload binding constraint");

    case ConstraintKind::Disjunction:
        llvm_unreachable("Disjunction constraints should use create()");

    case ConstraintKind::Conjunction:
        llvm_unreachable("Conjunction constraints should use create()");

    case ConstraintKind::SyntacticElement:
        llvm_unreachable("Syntactic element constraint should use create()");
    }
    std::uninitialized_copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

Constraint::Constraint(
    ConstraintKind kind, ConversionRestrictionKind restriction,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : _kind(kind)
    , _restriction(restriction)
    , _numTypeVariables(typeVars.size())
    , _hasFix(false)
    , _hasRestriction(true)
    , _isActive(false)
    , _isDisabled(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _types { first, second }
    , _locator(locator)
{
    assert(first && "First type is Null");
    assert(second && "Second type is Null");

    std::copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : _kind(kind)
    , _numTypeVariables(typeVars.size())
    , _hasFix(false)
    , _hasRestriction(false)
    , _isActive(false)
    , _isDisabled(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _member { first, second, member }
    , _locator(locator)
{
    assert(
        kind == ConstraintKind::ValueMember
        || kind == ConstraintKind::UnresolvedValueMember
    );
    assert(member && "Member constraint has no member");
    assert(locator && "Member constraint has no locator");

    std::copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

Constraint::Constraint(
    glu::ast::ASTNode node, bool isDiscarded, glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : _kind(ConstraintKind::SyntacticElement)
    , _numTypeVariables(typeVars.size())
    , _hasFix(false)
    , _hasRestriction(false)
    , _isActive(false)
    , _isDisabled(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _isDiscarded(isDiscarded)
    , _syntacticElement { node }
    , _locator(locator)
{
    std::copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

Constraint::Constraint(
    glu::types::Ty type, glu::ast::FunctionDecl *choice,
    glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : _kind(ConstraintKind::BindOverload)
    , _numTypeVariables(typeVars.size())
    , _hasRestriction(false)
    , _isActive(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _overload { type , choice }
    , _locator(locator)
{
    std::copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

static void getTypeVariable(
    glu::types::Ty ty,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
{
    if (auto *var = llvm::dyn_cast<glu::types::TypeVariableTy>(ty))
        typeVars.insert(var);
}

/// Recursively gather the set of type variables referenced by this constraint.
static void gatherReferencedTypeVars(
    Constraint *constraint,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
{
    switch (constraint->getKind()) {
    case ConstraintKind::Disjunction:
        for (auto nested : constraint->getNestedConstraints())
            gatherReferencedTypeVars(nested, typeVars);
        return;

    case ConstraintKind::Conjunction:
        typeVars.insert(
            constraint->getTypeVariables().begin(),
            constraint->getTypeVariables().end()
        );
        return;
    case ConstraintKind::Bind:
    case ConstraintKind::BindToPointerType:
    case ConstraintKind::ArgumentConversion:
    case ConstraintKind::Conversion:
    case ConstraintKind::OperatorArgumentConversion:
    case ConstraintKind::CheckedCast:
    case ConstraintKind::Equal:
    case ConstraintKind::UnresolvedValueMember:
    case ConstraintKind::ValueMember:
    case ConstraintKind::Defaultable:
    case ConstraintKind::ExplicitGenericArguments:
    case ConstraintKind::LValueObject:
        getTypeVariable(constraint->getFirstType(), typeVars);
        getTypeVariable(constraint->getSecondType(), typeVars);

        break;

    case ConstraintKind::BindOverload:
        getTypeVariable(constraint->getFirstType(), typeVars);

        // // Special case: the base type of an overloading binding.
        getTypeVariable(constraint->getOverloadChoice()->getType(), typeVars);
        break;

    case ConstraintKind::SyntacticElement:
        typeVars.insert(
            constraint->getTypeVariables().begin(),
            constraint->getTypeVariables().end()
        );
        break;
    }
}

Constraint *Constraint::create(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator,
    llvm::ArrayRef<glu::types::TypeVariableTy *> extraTypeVars
)
{
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;

    assert(first && "First type is Null");
    assert(second && "Second type is Null");
    assert(locator && "Locator is Null");

    getTypeVariable(first, typeVars);
    getTypeVariable(second, typeVars);

    typeVars.insert(extraTypeVars.begin(), extraTypeVars.end());

    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    return ::new (mem) Constraint(kind, first, second, locator, typeVars);
}

Constraint *Constraint::createMember(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    glu::types::Ty first, glu::types::Ty second,
    glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
)
{
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;

    getTypeVariable(first, typeVars);
    getTypeVariable(second, typeVars);

    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    return new (mem) Constraint(kind, first, second, member, locator, typeVars);
}

Constraint *Constraint::createSyntacticElement(
    glu::types::Ty var, llvm::BumpPtrAllocator &allocator,
    glu::ast::ASTNode node, glu::ast::ASTNode *locator, bool isDiscarded
)
{
    // Collect type variables.
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;

    getTypeVariable(var, typeVars);

    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    return new (mem) Constraint(node, isDiscarded, locator, typeVars);
}

Constraint *Constraint::createConjunction(
    llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator,
    llvm::ArrayRef<glu::types::TypeVariableTy *> referencedVars
)
{
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;
    typeVars.insert(referencedVars.begin(), referencedVars.end());

    // Conjunctions don't gather constraints from either elements
    // because each have to be solved in isolation.

    assert(!constraints.empty() && "Empty conjunction constraint");
    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    auto conjunction = new (mem)
        Constraint(ConstraintKind::Conjunction, constraints, locator, typeVars);
    return conjunction;
}

Constraint *Constraint::createRestricted(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    ConversionRestrictionKind restriction, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    // Collect type variables.
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;
    getTypeVariable(first, typeVars);
    getTypeVariable(second, typeVars);

    // Create the constraint.
    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    return new (mem)
        Constraint(kind, restriction, first, second, locator, typeVars);
}

Constraint *Constraint::createBindOverload(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
    glu::ast::FunctionDecl *choice, glu::ast::ASTNode *locator
)
{
    // Collect type variables.
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;

    getTypeVariable(type, typeVars);
    getTypeVariable(choice->getType(), typeVars);

    // Create the constraint.
    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    return new (mem) Constraint(type, choice, locator, typeVars);
}

Constraint *Constraint::createDisjunction(
    llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator, bool rememberChoice
)
{
    // Unwrap any disjunctions inside the disjunction constraint; we only allow
    // disjunctions at the top level.
    llvm::SmallPtrSet<glu::types::TypeVariableTy *, 4> typeVars;
    bool unwrappedAny = false;
    llvm::SmallVector<Constraint *, 1> unwrapped;
    unsigned index = 0;
    for (auto constraint : constraints) {
        // Gather type variables from this constraint.
        gatherReferencedTypeVars(constraint, typeVars);

        // If we have a nested disjunction, unwrap it.
        if (constraint->getKind() == ConstraintKind::Disjunction) {
            // If we haven't unwrapped anything before, copy all of the
            // constraints we skipped.
            if (!unwrappedAny) {
                unwrapped.append(
                    constraints.begin(), constraints.begin() + index
                );
                unwrappedAny = true;
            }

            // Add all of the constraints in the disjunction.
            unwrapped.append(
                constraint->getNestedConstraints().begin(),
                constraint->getNestedConstraints().end()
            );
        } else if (unwrappedAny) {
            // Since we unwrapped constraints before, add this constraint.
            unwrapped.push_back(constraint);
        }
        ++index;
    }

    // If we unwrapped anything, our list of constraints is the unwrapped list.
    if (unwrappedAny)
        constraints = unwrapped;

    assert(!constraints.empty() && "Empty disjunction constraint");

    // If there is a single constraint, this isn't a disjunction at all.
    if (constraints.size() == 1) {
        assert(!rememberChoice && "simplified an important disjunction?");
        return constraints.front();
    }

    // Create the disjunction constraint.
    auto size = totalSizeToAlloc<
        glu::types::TypeVariableTy *>(
        typeVars.size()
    );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    auto disjunction = new (mem)
        Constraint(ConstraintKind::Disjunction, constraints, locator, typeVars);
    disjunction->_rememberChoice = (bool) rememberChoice;
    return disjunction;
}

Constraint *Constraint::createMemberOrOuterDisjunction(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    glu::types::Ty first, glu::types::Ty second,
    glu::ast::StructMemberExpr *member,
    llvm::ArrayRef<glu::ast::FunctionDecl *> outerAlternatives,
    glu::ast::ASTNode *locator
)
{
    auto memberConstraint
        = createMember(allocator, kind, first, second, member, locator);

    if (outerAlternatives.empty())
        return memberConstraint;

    llvm::SmallVector<Constraint *, 4> constraints;
    constraints.push_back(memberConstraint);
    memberConstraint->setFavored(true);
    for (auto choice : outerAlternatives) {
        constraints.push_back(
            Constraint::createBindOverload(allocator, first, choice, locator)
        );
    }
    return Constraint::createDisjunction(
        allocator, constraints, locator, false
    );
}

} // namespace glu::sema
