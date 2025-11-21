#ifndef GLU_AST_TYPES_TYPEMACROS_HPP
#define GLU_AST_TYPES_TYPEMACROS_HPP

#include <llvm/ADT/ArrayRef.h>

#define GLU_TYPE_GEN_TRAILING(Type, TrailingType, CountExpr, GetterName)   \
    size_t numTrailingObjects(                                             \
        typename TrailingArgs::OverloadToken<TrailingType>                 \
    ) const                                                                \
    {                                                                      \
        return (CountExpr);                                                \
    }                                                                      \
                                                                           \
    llvm::ArrayRef<TrailingType> GetterName() const                        \
    {                                                                      \
        return llvm::ArrayRef<TrailingType>(                               \
            this->template getTrailingObjects<TrailingType>(), (CountExpr) \
        );                                                                 \
    }

#endif // GLU_AST_TYPES_TYPEMACROS_HPP
