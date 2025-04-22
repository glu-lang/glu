#include "Constraints.hpp"
#include <llvm/ADT/ArrayRef.h>

namespace glu::sema {

Constraint::Constraint(
    ConstraintKind kind, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator
)
    : _kind(kind)
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
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::ast::ASTNode *locator
)
    : _kind(kind)
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
    case ConstraintKind::GenericArguments:
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
    }
}

Constraint::Constraint(
    ConstraintKind kind, ConversionRestrictionKind restriction,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
)
    : _kind(kind)
    , _restriction(restriction)
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
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
)
    : _kind(kind)
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
}

Constraint::Constraint(
    glu::types::Ty type, glu::ast::FunctionDecl *choice,
    glu::ast::ASTNode *locator
)
    : _kind(ConstraintKind::BindOverload)
    , _hasRestriction(false)
    , _isActive(false)
    , _rememberChoice(false)
    , _isFavored(false)
    , _overload { type, choice }
    , _locator(locator)
{
}

Constraint *Constraint::create(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    assert(first && "First type is Null");
    assert(second && "Second type is Null");
    assert(locator && "Locator is Null");

    return ::new (allocator) Constraint(kind, first, second, locator);
}

Constraint *Constraint::createBind(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(allocator, ConstraintKind::Bind, first, second, locator);
}

Constraint *Constraint::createEqual(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(allocator, ConstraintKind::Equal, first, second, locator);
}

Constraint *Constraint::createBindToPointerType(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::BindToPointerType, first, second, locator
    );
}

Constraint *Constraint::createConversion(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::Conversion, first, second, locator
    );
}

Constraint *Constraint::createConversion(
    llvm::BumpPtrAllocator &allocator, ConversionRestrictionKind restriction,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return createRestricted(
        allocator, ConstraintKind::Conversion, restriction, first, second,
        locator
    );
}

Constraint *Constraint::createArgumentConversion(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::ArgumentConversion, first, second, locator
    );
}

Constraint *Constraint::createArgumentConversion(
    llvm::BumpPtrAllocator &allocator, ConversionRestrictionKind restriction,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return createRestricted(
        allocator, ConstraintKind::ArgumentConversion, restriction, first,
        second, locator
    );
}

Constraint *Constraint::createOperatorArgumentConversion(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::OperatorArgumentConversion, first, second,
        locator
    );
}

Constraint *Constraint::createOperatorArgumentConversion(
    llvm::BumpPtrAllocator &allocator, ConversionRestrictionKind restriction,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return createRestricted(
        allocator, ConstraintKind::OperatorArgumentConversion, restriction,
        first, second, locator
    );
}

Constraint *Constraint::createCheckedCast(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::CheckedCast, first, second, locator
    );
}

Constraint *Constraint::createCheckedCast(
    llvm::BumpPtrAllocator &allocator, ConversionRestrictionKind restriction,
    glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return createRestricted(
        allocator, ConstraintKind::CheckedCast, restriction, first, second,
        locator
    );
}

Constraint *Constraint::createDefaultable(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::Defaultable, first, second, locator
    );
}

Constraint *Constraint::createGenericArguments(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::GenericArguments, first, second, locator
    );
}

Constraint *Constraint::createLValueObject(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return create(
        allocator, ConstraintKind::LValueObject, first, second, locator
    );
}

Constraint *Constraint::createMember(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    glu::types::Ty first, glu::types::Ty second,
    glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
)
{
    return new (allocator) Constraint(kind, first, second, member, locator);
}

Constraint *Constraint::createConjunction(
    llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator
)
{
    auto conjunction = new (allocator)
        Constraint(ConstraintKind::Conjunction, constraints, locator);
    return conjunction;
}

Constraint *Constraint::createRestricted(
    llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
    ConversionRestrictionKind restriction, glu::types::Ty first,
    glu::types::Ty second, glu::ast::ASTNode *locator
)
{
    return new (allocator)
        Constraint(kind, restriction, first, second, locator);
}

Constraint *Constraint::createBindOverload(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
    glu::ast::FunctionDecl *choice, glu::ast::ASTNode *locator
)
{
    return new (allocator) Constraint(type, choice, locator);
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
    llvm::MutableArrayRef<Constraint *> result(
        allocator.Allocate<Constraint *>(constraints.size()), constraints.size()
    );
    std::uninitialized_copy(
        constraints.begin(), constraints.end(), result.begin()
    );

    // Create the disjunction constraint.
    auto disjunction = new (allocator)
        Constraint(ConstraintKind::Disjunction, result, locator);
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
