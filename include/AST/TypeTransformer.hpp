#ifndef GLU_AST_TYPE_TRANSFORMER_HPP
#define GLU_AST_TYPE_TRANSFORMER_HPP

#include "AST/ASTContext.hpp"
#include "AST/Types.hpp"
#include "Basic/InternedMemoryArena.hpp"
#include "Basic/SourceLocation.hpp"

namespace glu::types {

template <typename Derived>
class TypeTransformerBase
    : public glu::types::TypeVisitor<Derived, glu::types::TypeBase *> {
protected:
    InternedMemoryArena<types::TypeBase> &_types;

public:
    TypeTransformerBase(glu::ast::ASTContext *context)
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

    // Make TypeVisitor functions visible to derived classes
    using glu::types::TypeVisitor<Derived, glu::types::TypeBase *>::visit;
};

} // namespace glu::types

#endif // GLU_AST_TYPE_TRANSFORMER_HPP
