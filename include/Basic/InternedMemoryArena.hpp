#ifndef GLU_INTERNEDMEMORYARENA_HPP
#define GLU_INTERNEDMEMORYARENA_HPP

#include "TypedMemoryArena.hpp"

#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/Support/Casting.h>

#include <type_traits>

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
        return obj->hash();
    }

    static unsigned getHashValue(Base const &obj) { return obj.hash(); }

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

    template <typename T, typename... Args> T *findInterned(Args &&...args)
    {

        llvm::BumpPtrAllocator tmpAllocator;

        T *tmp = this->template createWithAllocator<T>(
            tmpAllocator, std::forward<Args>(args)...
        );

        auto it = _internedSet.find_as(tmp);
        if (it != _internedSet.end())
            return llvm::cast<T>(*it);
        return nullptr;
    }

public:
    template <typename T, typename... Args> T *create(Args &&...args)
    {
        if (T *interned = findInterned<T>(std::forward<Args>(args)...))
            return interned;

        T *obj
            = static_cast<TypedMemoryArena<Base> *>(this)->template create<T>(
                std::forward<Args>(args)...
            );
        _internedSet.insert(obj);
        return obj;
    }
};

} // namespace glu

#endif // GLU_INTERNEDMEMORYARENA_HPP
