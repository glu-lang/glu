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
    glu::ast::ASTContext *_context;

public:
    TypeMappingVisitorBase(glu::ast::ASTContext *context)
        : _context(context) { }

    glu::types::TypeBase *visitTypeBase(glu::types::TypeBase *type)
    {
        return type;
    }

    glu::types::TypeBase *visitFunctionTy(glu::types::FunctionTy *type)
    {
        auto *returnType = visit(type->getReturnType());
        std::vector<glu::types::TypeBase *> paramTypes;

        for (auto *param : type->getParameters())
            paramTypes.push_back(static_cast<Derived *>(this)->visit(param));

        return _context->getTypesMemoryArena().create<glu::types::FunctionTy>(
            paramTypes, returnType
        );
    }

    glu::types::TypeBase *visitPointerTy(glu::types::PointerTy *type)
    {
        auto *pointee = static_cast<Derived *>(this)->visit(type->getPointee());
        return _context->getTypesMemoryArena().create<glu::types::PointerTy>(
            pointee
        );
    }

    glu::types::TypeBase *visitTypeAliasTy(glu::types::TypeAliasTy *type)
    {
        auto *wrapped
            = static_cast<Derived *>(this)->visit(type->getWrappedType());
        return _context->getTypesMemoryArena().create<glu::types::TypeAliasTy>(
            wrapped, type->getName(), type->getLocation()
        );
    }

    glu::types::TypeBase *visitStaticArrayTy(glu::types::StaticArrayTy *type)
    {
        auto *element
            = static_cast<Derived *>(this)->visit(type->getDataType());
        return _context->getTypesMemoryArena()
            .create<glu::types::StaticArrayTy>(element, type->getSize());
    }

    glu::types::TypeBase *visitDynamicArrayTy(glu::types::DynamicArrayTy *type)
    {
        auto *element
            = static_cast<Derived *>(this)->visit(type->getDataType());
        return _context->getTypesMemoryArena()
            .create<glu::types::DynamicArrayTy>(element);
    }

    // Make TypeVisitor and TypeMapper functions visible to derived classes
    using glu::types::TypeVisitor<Derived, glu::types::TypeBase *>::visit;
    using glu::sema::TypeMapper<Derived>::visit;

    glu::types::TypeBase *mapType(glu::types::TypeBase *type)
    {
        return static_cast<Derived *>(this)->visit(type);
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_TY_MAPPER_VISITOR_HPP
