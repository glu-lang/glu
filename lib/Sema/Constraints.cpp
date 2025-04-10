#include "Constraints.hpp"

namespace glu::sema {

Constraint::Constraint(
    ConstraintKind kind, llvm::ArrayRef<Constraint *> constraints,
    bool isIsolated, glu::ast::ASTNode *locator,
    llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
)
    : Kind(kind)
    , NumTypeVariables(typeVars.size())
    , HasFix(false)
    , HasDeclContext(false)
    , HasRestriction(false)
    , IsActive(false)
    , IsDisabled(false)
    , RememberChoice(false)
    , IsFavored(false)
    , Nested(constraints)
    , Locator(locator)
{
    assert(
        kind == ConstraintKind::Disjunction
        || kind == ConstraintKind::Conjunction
    );
    std::uninitialized_copy(
        typeVars.begin(), typeVars.end(), getTypeVariablesBuffer().begin()
    );
}

}
