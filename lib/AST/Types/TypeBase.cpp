#include "AST/CanonicalTypeTransformer.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"

namespace glu::types {

TypeBase *TypeBase::getCanonicalType(glu::ast::ASTContext &context)
{
    return glu::ast::CanonicalTypeTransformer(context).visit(this);
}

bool TypeBase::isTrivial() const
{
    class TrivialityChecker : public TypeVisitor<TrivialityChecker, bool> {
    public:
        // Most types are trivial by default
        bool visitTypeBase([[maybe_unused]] TypeBase *type) { return true; }
        bool visitStructTy(StructTy *type)
        {
            return type->getDecl()->isTrivial();
        }
        bool visitTypeAliasTy(TypeAliasTy *type)
        {
            return visit(type->getWrappedType());
        }
        bool visitDynamicArrayTy([[maybe_unused]] DynamicArrayTy *type)
        {
            return false;
        }
        bool visitTypeVariableTy([[maybe_unused]] TypeVariableTy *type)
        {
            return false; // Don't know yet, can't assume it's trivial
        }
        bool visitUnresolvedNameTy([[maybe_unused]] UnresolvedNameTy *type)
        {
            return false; // Don't know yet, can't assume it's trivial
        }
    };

    return TrivialityChecker().visit(const_cast<TypeBase *>(this));
}

} // namespace glu::types
