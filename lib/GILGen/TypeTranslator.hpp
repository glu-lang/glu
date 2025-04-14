#ifndef GLU_GILGEN_TYPE_TRANSLATOR_HPP
#define GLU_GILGEN_TYPE_TRANSLATOR_HPP

#include "GIL/Type.hpp"
#include "Types.hpp"

namespace glu::gilgen {

/// @brief TypeTranslator est un visiteur qui convertit les types AST en types
/// GIL.
class TypeTranslator : public types::TypeVisitor<TypeTranslator, gil::Type> {
public:
    TypeTranslator() { }

    gil::Type visitTypeBase([[maybe_unused]] types::TypeBase *type)
    {
        assert(false && "Not implemented");
    }

    gil::Type visitIntTy(types::IntTy *type);
    gil::Type visitFloatTy(types::FloatTy *type);
    gil::Type visitBoolTy(types::BoolTy *type);
    gil::Type visitCharTy(types::CharTy *type);
    gil::Type visitVoidTy(types::VoidTy *type);

    gil::Type visitPointerTy(types::PointerTy *type);
    gil::Type visitFunctionTy(types::FunctionTy *type);
    gil::Type visitStructTy(types::StructTy *type);
    gil::Type visitStaticArrayTy(types::StaticArrayTy *type);
    gil::Type visitDynamicArrayTy(types::DynamicArrayTy *type);

    gil::Type visitTypeAliasTy(types::TypeAliasTy *type);
    gil::Type visitEnumTy(types::EnumTy *type);
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_TYPE_TRANSLATOR_HPP
