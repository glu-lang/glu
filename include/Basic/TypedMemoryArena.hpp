#ifndef GLU_TYPEDMEMORYARENA_HPP
#define GLU_TYPEDMEMORYARENA_HPP

#include "MemoryArena.hpp"

namespace glu {

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

template <typename T, typename... Args>
using CreateReturnStatic = std::enable_if_t<
    has_static_create<T, llvm::BumpPtrAllocator &, Args...>::value, T *>;

template <typename T, typename... Args>
using CreateReturnNonStatic = std::enable_if_t<
    !has_static_create<T, llvm::BumpPtrAllocator &, Args...>::value, T *>;

/// @brief A memory arena that ensures allocated objects are subclasses of a
/// specified base class.
///
/// @tparam Base The base class that all allocated objects must inherit from.
template <typename Base> class TypedMemoryArena : public MemoryArena {
public:
    // Overload if T has a static create method (for trailing objects, etc.)
    template <typename T, typename... Args>
    CreateReturnStatic<T, Args...> create(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>,
            "T must be a subclass of the base class used by the memory arena"
        );
        return T::create(this->getAllocator(), std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    CreateReturnStatic<T, Args...>
    create(llvm::BumpPtrAllocator &allocator, Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>,
            "T must be a subclass of the base class used by the memory arena"
        );

        return T::create(allocator, std::forward<Args>(args)...);
    }

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
    template <typename T, typename... Args>
    CreateReturnNonStatic<T, Args...> create(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>,
            "T must be a subclass of the base class used by the memory arena"
        );
        return allocate<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    CreateReturnNonStatic<T, Args...>
    create(llvm::BumpPtrAllocator &allocator, Args &&...args)
    {
        static_assert(
            std::is_base_of_v<Base, T>,
            "T must be a subclass of the base class used by the memory arena"
        );
        void *mem = allocator.Allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }
};

} // namespace glu

#endif // GLU_TYPEDMEMORYARENA_HPP
