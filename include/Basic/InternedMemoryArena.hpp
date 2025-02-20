#ifndef GLU_INTERNEDMEMORYARENA_HPP
#define GLU_INTERNEDMEMORYARENA_HPP

#include "TypedMemoryArena.hpp"

#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/Support/Casting.h>

#include <type_traits>
#include <utility>

template <typename T, typename Alloc, typename... Args>
class has_static_create {
private:
    template <typename U>
    static auto test(
        int
    ) -> decltype(U::create(std::declval<Alloc &>(), std::declval<Args>()...), std::true_type {});
    template <typename> static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

namespace glu {

template <typename Base> struct BaseDenseSetInternInfo {
    static inline Base *getEmptyKey() { return reinterpret_cast<Base *>(-1); }
    static inline Base *getTombstoneKey()
    {
        return reinterpret_cast<Base *>(-2);
    }

    static unsigned getHashValue(Base const *obj)
    {
        if (obj == getEmptyKey() || obj == getTombstoneKey())
            return 0;
        return obj->hash(obj);
    }

    static unsigned getHashValue(Base const &obj) { return obj.hashType(&obj); }

    static bool isEqual(Base const *lhs, Base const *rhs)
    {
        if (lhs == getEmptyKey() || lhs == getTombstoneKey()
            || rhs == getEmptyKey() || rhs == getTombstoneKey())
            return lhs == rhs;
        return *lhs == *rhs;
    }

    static bool isEqual(Base const *lhs, Base const &rhs)
    {
        if (lhs == getEmptyKey() || lhs == getTombstoneKey())
            return false;
        return *lhs == rhs;
    }
};

template <typename Base>
class InternedMemoryArena : public TypedMemoryArena<Base> {
    llvm::DenseSet<Base *, BaseDenseSetInternInfo<Base>> _internedSet;

public:
    template <typename T, typename... Args>
    std::enable_if_t<
        !has_static_create<T, llvm::BumpPtrAllocator, Args...>::value, T *>
    create(Args &&...args)
    {
        T tmp(std::forward<Args>(args)...);

        auto it = _internedSet.find_as(&tmp);
        if (it != _internedSet.end())
            return llvm::cast<T>(*it);

        T *obj
            = static_cast<TypedMemoryArena<Base> *>(this)->template create<T>(
                std::forward<Args>(args)...
            );

        _internedSet.insert(obj);
        return obj;
    }

    template <typename T, typename... Args>
    std::enable_if_t<
        has_static_create<T, llvm::BumpPtrAllocator, Args...>::value, T *>
    create(Args &&...args)
    {
        T *obj = T::create(this->getAllocator(), std::forward<Args>(args)...);

        auto result = _internedSet.insert(obj);
        if (!result.second) {
            obj = llvm::cast<T>(*result.first);
        }

        return obj;
    }
};

} // namespace glu

#endif // GLU_INTERNEDMEMORYARENA_HPP
