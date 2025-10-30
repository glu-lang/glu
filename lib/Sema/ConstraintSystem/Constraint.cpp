#include "Constraint.hpp"
#include <llvm/ADT/ArrayRef.h>

namespace glu::sema {

Constraint::Constraint(
    ConstraintKind kind, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator
)
    : _kind(kind), _nested(constraints), _locator(locator)
{
    assert(
        kind == ConstraintKind::Disjunction
        || kind == ConstraintKind::Conjunction
    );
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty type, glu::ast::ASTNode *locator
)
    : _kind(kind), _singleType(type), _locator(locator)
{
    assert(type && "Type is Null");
    assert(
        (kind == ConstraintKind::ExpressibleByIntLiteral
         || kind == ConstraintKind::ExpressibleByStringLiteral
         || kind == ConstraintKind::ExpressibleByFloatLiteral
         || kind == ConstraintKind::ExpressibleByBoolLiteral
         || kind == ConstraintKind::StructInitialiser)
        && "Should be ExpressibleByLiteral or StructInitialiser constraint"
    );
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::ast::ASTNode *locator
)
    : _kind(kind), _types { first, second }, _locator(locator)
{
    assert(first && "First type is Null");
    assert(second && "Second type is Null");

    switch (_kind) {
    case ConstraintKind::Bind:
    case ConstraintKind::Equal:
    case ConstraintKind::BindToPointerType:
    case ConstraintKind::Conversion:
    case ConstraintKind::CheckedCast: break;
    case ConstraintKind::ValueMember:
    case ConstraintKind::Defaultable: break;

    case ConstraintKind::BindOverload:
        llvm_unreachable("Wrong constructor for overload binding constraint");

    case ConstraintKind::Disjunction:
        llvm_unreachable("Disjunction constraints should use create()");

    case ConstraintKind::Conjunction:
        llvm_unreachable("Conjunction constraints should use create()");

    case ConstraintKind::ExpressibleByIntLiteral:
    case ConstraintKind::ExpressibleByStringLiteral:
    case ConstraintKind::ExpressibleByFloatLiteral:
    case ConstraintKind::ExpressibleByBoolLiteral:
        llvm_unreachable(
            "Wrong constructor for ExpressibleByLiteral constraint"
        );
    default: llvm_unreachable("Unsupported constraint kind");
    }
}

Constraint::Constraint(
    ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
    glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
)
    : _kind(kind), _member { first, second, member }, _locator(locator)
{
    assert(kind == ConstraintKind::ValueMember);
    assert(member && "Member constraint has no member");
    assert(locator && "Member constraint has no locator");
}

Constraint::Constraint(
    glu::types::Ty type, glu::ast::FunctionDecl *choice,
    glu::ast::ASTNode *locator
)
    : _kind(ConstraintKind::BindOverload)
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

    return new (allocator) Constraint(kind, first, second, locator);
}

Constraint *Constraint::createDisjunction(
    llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator, bool rememberChoice
)
{
    // Unwrap any disjunctions inside the disjunction constraint; we only allow
    // disjunctions at the top level.
    llvm::SmallVector<Constraint *, 4> unwrapped;

    for (auto constraint : constraints) {
        if (constraint->getKind() == ConstraintKind::Disjunction) {
            unwrapped.append(
                constraint->getNestedConstraints().begin(),
                constraint->getNestedConstraints().end()
            );
        } else {
            unwrapped.push_back(constraint);
        }
    }

    assert(!unwrapped.empty() && "Empty disjunction constraint");

    // If there is a single constraint, this isn't a disjunction at all.
    if (unwrapped.size() == 1) {
        assert(!rememberChoice && "simplified an important disjunction?");
        return unwrapped.front();
    }
    llvm::MutableArrayRef<Constraint *> nested(
        allocator.Allocate<Constraint *>(unwrapped.size()), unwrapped.size()
    );
    std::uninitialized_copy(unwrapped.begin(), unwrapped.end(), nested.begin());

    // Create the disjunction constraint.
    auto disjunction = new (allocator)
        Constraint(ConstraintKind::Disjunction, nested, locator);
    return disjunction;
}

Constraint *Constraint::createConjunction(
    llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<Constraint *> constraints,
    glu::ast::ASTNode *locator
)
{
    // Unwrap any conjunctions inside the conjunction constraint; we only allow
    // conjunctions at the top level.
    llvm::SmallVector<Constraint *, 4> unwrapped;

    for (auto constraint : constraints) {
        if (constraint->getKind() == ConstraintKind::Conjunction) {
            unwrapped.append(
                constraint->getNestedConstraints().begin(),
                constraint->getNestedConstraints().end()
            );
        } else {
            unwrapped.push_back(constraint);
        }
    }

    assert(!unwrapped.empty() && "Empty conjunction constraint");

    // If there is a single constraint, this isn't a conjunction at all.
    if (unwrapped.size() == 1) {
        return unwrapped.front();
    }
    llvm::MutableArrayRef<Constraint *> nested(
        allocator.Allocate<Constraint *>(unwrapped.size()), unwrapped.size()
    );
    std::uninitialized_copy(unwrapped.begin(), unwrapped.end(), nested.begin());

    // Create the disjunction constraint.
    auto conjunction = new (allocator)
        Constraint(ConstraintKind::Conjunction, nested, locator);
    return conjunction;
}

Constraint *Constraint::createExpressibleByLiteral(
    llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
    glu::ast::ASTNode *locator, ConstraintKind kind
)
{
    return new (allocator) Constraint(kind, type, locator);
}

} // namespace glu::sema
