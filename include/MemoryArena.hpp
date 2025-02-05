#ifndef GLU_MEMORYARENA_HPP
#define GLU_MEMORYARENA_HPP

#include <llvm/Support/Allocator.h>

namespace glu::ast {
class MemoryArena {
    llvm::BumpPtrAllocator allocator;

public:
    MemoryArena() = default;
    ~MemoryArena() = default;

    template <typename T, typename... Args> T *allocate(Args &&...args)
    {
        void *mem = allocator.Allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    template <typename T> void deallocate(T *ptr)
    {
        ptr->~T();
        allocator.Deallocate(ptr);
    }
};
}; // namespace glu::ast

#endif // GLU_MEMORYARENA_HPP
