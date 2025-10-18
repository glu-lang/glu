#include "AST/Types.hpp"
#include "Constraint.hpp"

#include <llvm/ADT/DenseSet.h>

namespace glu::sema {

class TypeVariableCollector
    : public glu::types::TypeVisitor<TypeVariableCollector, void> {
protected:
    llvm::DenseSet<glu::types::TypeVariableTy *> &_typeVariables;

public:
    TypeVariableCollector(
        llvm::DenseSet<glu::types::TypeVariableTy *> &typeVariables
    )
        : _typeVariables(typeVariables)
    {
    }

    void visitTypeBase([[maybe_unused]] glu::types::TypeBase *type) { }

    void visitFunctionTy(glu::types::FunctionTy *type)
    {
        visit(type->getReturnType());
        for (glu::types::TypeBase *paramType : type->getParameters())
            visit(paramType);
    }

    void visitPointerTy(types::PointerTy *type) { visit(type->getPointee()); }

    void visitTypeAliasTy(types::TypeAliasTy *type)
    {
        visit(type->getWrappedType());
    }

    void visitStaticArrayTy(types::StaticArrayTy *type)
    {
        visit(type->getDataType());
    }

    void visitDynamicArrayTy(types::DynamicArrayTy *type)
    {
        visit(type->getDataType());
    }

    void visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        _typeVariables.insert(type);
    }
};

void collectTypeVariables(
    Constraint *constraint,
    llvm::DenseSet<glu::types::TypeVariableTy *> &typeVars
)
{
    if (constraint->getKind() == ConstraintKind::Disjunction
        || constraint->getKind() == ConstraintKind::Conjunction) {
        for (Constraint *nested : constraint->getNestedConstraints()) {
            collectTypeVariables(nested, typeVars);
        }
        return;
    }
    TypeVariableCollector collector(typeVars);
    collector.visit(constraint->getFirstType());
    if (!constraint->isTypePropertyConstraint()
        && constraint->getKind() != ConstraintKind::BindOverload
        && constraint->getKind() != ConstraintKind::StructInitialiser)
        collector.visit(constraint->getSecondType());
}

} // namespace glu::sema
