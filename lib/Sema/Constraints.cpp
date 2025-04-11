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
    , _types { first, second, glu::types::Ty() }
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
    case ConstraintKind::FallbackType: break;

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
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::types::Ty third, glu::ast::ASTNode *locator,
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
    , _types { first, second, third }
    , _locator(locator)
{
    assert(first && "First type is Null");
    assert(second && "Second type is Null");
    assert(third && "Third type is Null");

    switch (_kind) {
    case ConstraintKind::Bind:
    case ConstraintKind::Equal:
    case ConstraintKind::BindToPointerType:
    case ConstraintKind::Conversion:
    case ConstraintKind::ArgumentConversion:
    case ConstraintKind::OperatorArgumentConversion:
    case ConstraintKind::CheckedCast:
    case ConstraintKind::ValueMember:
    case ConstraintKind::UnresolvedValueMember:
    case ConstraintKind::Defaultable:
    case ConstraintKind::BindOverload:
    case ConstraintKind::Disjunction:
    case ConstraintKind::Conjunction:
    case ConstraintKind::FallbackType:
    case ConstraintKind::SyntacticElement:
    case ConstraintKind::ExplicitGenericArguments:
    case ConstraintKind::LValueObject: llvm_unreachable("Wrong constructor");
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
    , _types { first, second, glu::types::Ty() }
    , _locator(locator)
{
    assert(first && "First type is Null");
    assert(second && "Second type is Null");

    std::copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
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

    if (first->getKind() == glu::types::TypeKind::TypeVariableTyKind)
        typeVars.insert(llvm::cast<glu::types::TypeVariableTy>(first));

    if (second->getKind() == glu::types::TypeKind::TypeVariableTyKind)
        typeVars.insert(llvm::cast<glu::types::TypeVariableTy>(second));

    typeVars.insert(extraTypeVars.begin(), extraTypeVars.end());

    auto size
        = totalSizeToAlloc<glu::types::TypeVariableTy *, glu::ast::DeclBase *>(
            typeVars.size(), 1
        );
    void *mem = allocator.Allocate(size, alignof(Constraint));
    return ::new (mem) Constraint(kind, first, second, locator, typeVars);
}
}
