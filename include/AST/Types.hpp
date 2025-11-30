#ifndef GLU_AST_TYPES_HPP
#define GLU_AST_TYPES_HPP

#include "Types/BoolTy.hpp"
#include "Types/CharTy.hpp"
#include "Types/DynamicArrayTy.hpp"
#include "Types/EnumTy.hpp"
#include "Types/FloatTy.hpp"
#include "Types/FunctionTy.hpp"
#include "Types/IntTy.hpp"
#include "Types/NullTy.hpp"
#include "Types/PointerTy.hpp"
#include "Types/StaticArrayTy.hpp"
#include "Types/StructTy.hpp"
#include "Types/TemplateParamTy.hpp"
#include "Types/TypeAliasTy.hpp"
#include "Types/TypeVariableTy.hpp"
#include "Types/UnresolvedNameTy.hpp"
#include "Types/VoidTy.hpp"

// Include the TypeVisitor after all the types
#include "Types/TypeVisitor.hpp"

namespace glu::types {

/// @brief Extract the underlying FunctionTy from a type.
/// @param type The type to extract from.
/// @return The FunctionTy if the type is a FunctionTy or a PointerTy to a
/// FunctionTy, nullptr otherwise.
inline FunctionTy *getUnderlyingFunctionTy(TypeBase *type)
{
    if (auto *funcTy = llvm::dyn_cast<FunctionTy>(type)) {
        return funcTy;
    }
    if (auto *ptrTy = llvm::dyn_cast<PointerTy>(type)) {
        return llvm::dyn_cast<FunctionTy>(ptrTy->getPointee());
    }
    return nullptr;
}

} // namespace glu::types

#endif // GLU_AST_TYPES_HPP
