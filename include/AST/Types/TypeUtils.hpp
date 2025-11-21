#ifndef GLU_AST_TYPES_TYPEUTILS_HPP
#define GLU_AST_TYPES_TYPEUTILS_HPP

#include "AST/Types.hpp"

#include <llvm/Support/Casting.h>

namespace glu::types {

/// @brief Returns the underlying FunctionTy if the given type is a function
/// type or a pointer to a function type.
inline FunctionTy *getUnderlyingFunctionType(TypeBase *type)
{
    if (auto *fnTy = llvm::dyn_cast_if_present<FunctionTy>(type)) {
        return fnTy;
    }

    if (auto *ptrTy = llvm::dyn_cast_if_present<PointerTy>(type)) {
        return llvm::dyn_cast_if_present<FunctionTy>(ptrTy->getPointee());
    }

    return nullptr;
}

} // namespace glu::types

#endif // GLU_AST_TYPES_TYPEUTILS_HPP
