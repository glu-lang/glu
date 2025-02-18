#ifndef GLU_TYPEDMEMORYARENA_HPP
#define GLU_TYPEDMEMORYARENA_HPP

#include "MemoryArena.hpp"

namespace glu {

/// @brief A memory arena that ensures allocated objects are subclasses of a
/// specified base class.
///
/// @tparam Base The base class that all allocated objects must inherit from.
template <typename Base> class TypedMemoryArena : public MemoryArena {
public:
    /// @brief Creates and allocates an object of type T within the memory
    /// arena.
    ///
    /// @tparam T The type of the object to create. Must be a subclass of Base.
    /// @tparam Args The types of the arguments to pass to the constructor of T.
    /// @param args The arguments to pass to the constructor of T.
    /// @return A pointer to the newly created object of type T.
    ///
    /// @note This function uses static_assert to ensure that T is a subclass of
    /// Base.
    /// @note This function forwards the provided arguments to the constructor
    /// of T.
    template <typename T, typename... Args> T *create(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>,
            "T must be a subclass of the base class used by the memory arena"
        );

        return allocate<T>(std::forward<Args>(args)...);
    }
};

} // namespace glu

#endif // GLU_TYPEDMEMORYARENA_HPP
