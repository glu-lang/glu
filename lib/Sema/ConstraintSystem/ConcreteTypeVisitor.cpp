#include "ConstraintSystem.hpp"

namespace glu::sema {

/// @brief A visitor that checks if a type variable occurs within a type.
///
/// This implements the "occurs check" which prevents the creation of infinite
/// types like T = List<T>. It recursively traverses composite types to ensure
/// that a type variable doesn't appear within the type it's being bound to.
class ConcreteTypeVisitor
    : public glu::types::TypeVisitor<ConcreteTypeVisitor, bool> {
public:
    ConcreteTypeVisitor() = default;

    bool visitTypeBase(glu::types::TypeBase * /*type*/) { return true; }

    bool visitTypeVariableTy([[maybe_unused]] glu::types::TypeVariableTy *type)
    {
        return false; // Type variables are not concrete
    }

    bool visitPointerTy(glu::types::PointerTy *type)
    {
        return visit(type->getPointee());
    }

    bool visitFunctionTy(glu::types::FunctionTy *type)
    {
        if (!visit(type->getReturnType()))
            return false;
        for (auto param : type->getParameters()) {
            if (!visit(param))
                return false;
        }
        return true;
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

bool typeIsConcrete(glu::types::Ty type)
{
    return ConcreteTypeVisitor().visit(type);
}

} // namespace glu::sema
