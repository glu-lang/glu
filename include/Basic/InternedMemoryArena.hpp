#ifndef GLU_INTERNEDMEMORYARENA_HPP
#define GLU_INTERNEDMEMORYARENA_HPP

#include "TypedMemoryArena.hpp"
#include "Types/TypeBase.hpp"

#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/Support/Casting.h>

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

template <typename Base> class InternedMemoryArena {
    TypedMemoryArena<Base> _arena;
    llvm::DenseSet<Base *, BaseDenseSetInternInfo<Base>> _internedSet;

public:
    template <typename T, typename... Args> T *create(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>, "T must be a subclass of Base"
        );

        T tmp(std::forward<Args>(args)...);

        auto it = _internedSet.find_as(&tmp);
        if (it != _internedSet.end())
            return llvm::cast<T>(*it);

        T *obj = _arena.template create<T>(std::forward<Args>(args)...);
        _internedSet.insert(obj);
        return obj;
    }
};

} // namespace glu

#endif // GLU_INTERNEDMEMORYARENA_HPP
