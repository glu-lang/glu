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

/// @brief Substitutes type variables with their bindings in a type.
/// @param type The type to substitute.
/// @param bindings The current type variable bindings.
/// @param context The AST context to create new types if needed.
/// @return The type with substitutions applied.
glu::types::Ty substitute(
    glu::types::Ty type,
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *> const
        &bindings,
    glu::ast::ASTContext *context
)
{
    SubstitutionMapper mapper(context, bindings);
    return mapper.visit(type);
}

} // namespace glu::sema
