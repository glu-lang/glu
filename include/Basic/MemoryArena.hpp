#ifndef GLU_MEMORYARENA_HPP
#define GLU_MEMORYARENA_HPP

#include <llvm/Support/Allocator.h>

namespace glu {

/// @brief A memory arena that can be used to allocate memory.
class MemoryArena {
    llvm::BumpPtrAllocator allocator;

public:
    MemoryArena() = default;
    ~MemoryArena() = default;

    /// @brief Get the allocator used by the memory arena.
    /// @return The allocator used by the memory arena.
    llvm::BumpPtrAllocator &getAllocator() { return allocator; }

    /// @brief Allocate memory in the memory arena.
    /// @return A pointer to the allocated memory.
    template <typename T, typename... Args> T *allocate(Args &&...args)
    {
        void *mem = allocator.Allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }
};

}; // namespace glu

#endif // GLU_MEMORYARENA_HPP
