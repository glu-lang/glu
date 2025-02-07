#ifndef GLU_TYPEDMEMORYARENA_HPP
#define GLU_TYPEDMEMORYARENA_HPP

#include "MemoryArena.hpp"
#include <type_traits>

namespace glu {

template <typename Base> class TypedMemoryArena : public MemoryArena {
public:
    template <typename T, typename... Args> T *create(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>,
            "T must be a subclass of the base class used by the memory arena"
        );
        return allocate<T>(std::forward<Args>(args)...);
    }
};

}; // namespace glu

#endif // GLU_TYPEDMEMORYARENA_HPP
