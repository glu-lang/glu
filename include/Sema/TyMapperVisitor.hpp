#ifndef GLU_SEMA_TY_MAPPER_VISITOR_HPP
#define GLU_SEMA_TY_MAPPER_VISITOR_HPP

#include "Basic/Diagnostic.hpp"
#include "ConstraintSystem.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

/// A reusable type mapping base for substituting/rewriting types using a
/// solution.
template <typename Derived>
class TypeMappingVisitorBase
    : public glu::sema::TypeMapper<Derived>,
      public glu::types::TypeVisitor<Derived, glu::types::TypeBase *> {
protected:
    InternedMemoryArena<types::TypeBase> &_types;

public:
    TypeMappingVisitorBase(glu::ast::ASTContext *context)
        : _types(context->getTypesMemoryArena())
    {
    }

    glu::types::TypeBase *visitTypeBase(glu::types::TypeBase *type)
    {
        return type;
    }

    glu::types::TypeBase *visitFunctionTy(glu::types::FunctionTy *type)
    {
        glu::types::TypeBase *returnType = visit(type->getReturnType());
        llvm::SmallVector<glu::types::TypeBase *, 4> params;

        for (glu::types::TypeBase *paramType : type->getParameters())
            params.push_back(visit(paramType));

        return _types.create<glu::types::FunctionTy>(params, returnType);
    }

    types::TypeBase *visitPointerTy(types::PointerTy *type)
    {
        glu::types::TypeBase *pointeeType = visit(type->getPointee());
        return _types.create<glu::types::PointerTy>(pointeeType);
    }

    types::TypeBase *visitTypeAliasTy(types::TypeAliasTy *type)
    {
        glu::types::TypeBase *aliasedType = visit(type->getWrappedType());
        return _types.create<glu::types::TypeAliasTy>(
            aliasedType, type->getName(), type->getLocation()
        );
    }

    types::TypeBase *visitStaticArrayTy(types::StaticArrayTy *type)
    {
        glu::types::TypeBase *elementType = visit(type->getDataType());
        return _types.create<glu::types::StaticArrayTy>(
            elementType, type->getSize()
        );
    }

    types::TypeBase *visitDynamicArrayTy(types::DynamicArrayTy *type)
    {
        glu::types::TypeBase *elementType = visit(type->getDataType());
        return _types.create<glu::types::DynamicArrayTy>(elementType);
    }

    // Make TypeVisitor and TypeMapper functions visible to derived classes
    using glu::types::TypeVisitor<Derived, glu::types::TypeBase *>::visit;
    using glu::sema::TypeMapper<Derived>::visit;

    glu::types::TypeBase *mapType(glu::types::TypeBase *type)
    {
        if (type == nullptr)
            return nullptr;
        return visit(type);
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_TY_MAPPER_VISITOR_HPP
