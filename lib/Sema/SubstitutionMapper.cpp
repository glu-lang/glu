#include "ConstraintSystem.hpp"
#include "TyMapperVisitor.hpp"

namespace glu::sema {

/// @brief A type mapper that substitutes type variables with their bindings.
///
/// This visitor traverses a type and replaces any type variables with their
/// corresponding bindings from the provided mapping. It handles recursive
/// substitution for chains like T1 -> T2 -> Int.
class SubstitutionMapper : public TypeMappingVisitorBase<SubstitutionMapper> {
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *> const
        &_bindings;

public:
    SubstitutionMapper(
        glu::ast::ASTContext *context,
        llvm::DenseMap<
            glu::types::TypeVariableTy *, glu::types::TypeBase *> const
            &bindings
    )
        : TypeMappingVisitorBase(context), _bindings(bindings)
    {
    }

    glu::types::TypeBase *visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        auto it = _bindings.find(type);
        if (it != _bindings.end()) {
            // Recursively substitute to handle chains like T1 -> T2 -> Int
            return visit(it->second);
        }
        return type;
    }
};

glu::types::Ty ConstraintSystem::substitute(
    glu::types::Ty type,
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *> const
        &bindings
)
{
    SubstitutionMapper mapper(_context, bindings);
    return mapper.visit(type);
}

} // namespace glu::sema
