#include "ConstraintSystem.hpp"

namespace glu::sema {

/// @brief A visitor that checks if a type variable occurs within a type.
///
/// This implements the "occurs check" which prevents the creation of infinite
/// types like T = List<T>. It recursively traverses composite types to ensure
/// that a type variable doesn't appear within the type it's being bound to.
class OccursCheckVisitor
    : public glu::types::TypeVisitor<OccursCheckVisitor, bool> {
    glu::types::TypeVariableTy *_var;
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *> const
        &_bindings;

public:
    OccursCheckVisitor(
        glu::types::TypeVariableTy *var,
        llvm::DenseMap<
            glu::types::TypeVariableTy *, glu::types::TypeBase *> const
            &bindings,
        glu::ast::ASTContext * /*context*/
    )
        : _var(var), _bindings(bindings)
    {
    }

    bool visitTypeBase(glu::types::TypeBase * /*type*/) { return false; }

    bool visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        if (type == _var)
            return true;

        auto it = _bindings.find(type);
        if (it != _bindings.end()) {
            return visit(it->second);
        }
        return false;
    }

    bool visitPointerTy(glu::types::PointerTy *type)
    {
        return visit(type->getPointee());
    }

    bool visitFunctionTy(glu::types::FunctionTy *type)
    {
        if (visit(type->getReturnType()))
            return true;
        for (auto param : type->getParameters()) {
            if (visit(param))
                return true;
        }
        return false;
    }

    bool visitStaticArrayTy(glu::types::StaticArrayTy *type)
    {
        return visit(type->getDataType());
    }

    bool visitDynamicArrayTy(glu::types::DynamicArrayTy *type)
    {
        return visit(type->getDataType());
    }

    // Add more cases as needed for other composite types
};

bool ConstraintSystem::occursCheck(
    glu::types::TypeVariableTy *var, glu::types::Ty type,
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *> const
        &bindings
)
{
    // Apply substitutions first
    type = substitute(type, bindings);

    if (type == var)
        return true;

    OccursCheckVisitor visitor(var, bindings, _context);
    return visitor.visit(type);
}

} // namespace glu::sema
