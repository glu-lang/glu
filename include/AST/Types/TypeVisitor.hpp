#ifndef GLU_AST_TYPES_TYPEVISITOR_HPP
#define GLU_AST_TYPES_TYPEVISITOR_HPP

#include "TypeBase.hpp"
#include <iostream>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>
#include <type_traits>

namespace glu::types {

template <typename T, typename... Args> struct has_beforeVisit {
    template <typename U>
    static auto test(
        int
    ) -> decltype(std::declval<U>().beforeVisit(std::declval<Args>()...), std::true_type {});

    template <typename> static auto test(...) -> std::false_type;

    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename Impl, typename RetTy = void, typename... ArgTys>
class TypeVisitor {
    Impl *asImpl() { return static_cast<Impl *>(this); }

public:
    /**
     * @brief Visit a Type.
     *
     * This function call the appropriate visit function for the type's kind.
     *
     * @param type The type to visit.
     * @param ...args The arguments to pass to the visit function.
     * @return The value returned by the visit function.
     *
     **/
    RetTy visit(TypeBase *type, ArgTys... args)
    {
        return _visitType(type, std::forward<ArgTys>(args)...);
    }

    RetTy _visitType(TypeBase *type, ArgTys... args)
    {
        // Check if the implementation has a beforeVisit method and call it
        if constexpr (std::is_same_v<RetTy, bool>
                      && has_beforeVisit<Impl, TypeBase *, ArgTys...>::value) {
            if (asImpl()->beforeVisit(type, std::forward<ArgTys>(args)...)) {
                return true;
            }
        }

        switch (type->getKind()) {
#define TYPE(NAME)                                            \
case TypeKind::NAME##Kind:                                    \
    return asImpl()->visit##NAME(                             \
        llvm::cast<NAME>(type), std::forward<ArgTys>(args)... \
    );
#include "TypeKind.def"
        default: llvm_unreachable("Unknown type kind.");
        }
    }

    RetTy visitTypeBase(
        [[maybe_unused]] TypeBase *type, [[maybe_unused]] ArgTys... args
    )
    {
        if constexpr (std::is_default_constructible_v<RetTy>) {
            return RetTy();
        } else {
            assert(
                false && "No default implementation provided for this type."
            );
        }
    }

#define TYPE(NAME)                                                           \
    RetTy visit##NAME(NAME *type, ArgTys... args)                            \
    {                                                                        \
        return asImpl()->visitTypeBase(type, std::forward<ArgTys>(args)...); \
    }
#include "TypeKind.def"
};

} // namespace glu::types

#endif /* !GLU_AST_TYPES_TYPEVISITOR_HPP */
