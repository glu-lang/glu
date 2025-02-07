#ifndef GLU_AST_ASTCONTEXT_HPP
#define GLU_AST_ASTCONTEXT_HPP

#include "ASTNode.hpp"
#include "Basic/MemoryArena.hpp"

namespace glu::ast {

class ASTContext {
    MemoryArena _arena;

public:
    ASTContext() = default;

    /// @brief Get the memory arena used by the AST context.
    /// @return The memory arena used by the AST context.
    MemoryArena &getMemoryArena() { return _arena; }

    /// @brief Get allocator used by the memory arena.
    /// @return The allocator used by the memory arena.
    llvm::BumpPtrAllocator &getAllocator() { return _arena.getAllocator(); }

    /// @brief Reset the memory arena.
    void reset() { _arena.reset(); }

    /// @brief Allocate one AST node in the memory arena.
    /// @tparam T The type of the AST node to allocate.
    /// @tparam Args The types of the arguments to pass to the constructor of
    /// the AST node.
    /// @param args The arguments to pass to the constructor of the AST node.
    /// @return A pointer to the allocated AST node.
    template <typename T, typename... Args> T *createNode(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<ASTNode, T>, "T must be a subclass of ASTNode"
        );
        return _arena.allocate<T>(std::forward<Args>(args)...);
    }
};

}; // namespace glu::ast

#endif /* GLU_AST_ASTCONTEXT_HPP */
