#ifndef GLU_AST_TYPESMEMORYARENA_HPP
#define GLU_AST_TYPESMEMORYARENA_HPP

#include "Basic/MemoryArena.hpp"
#include "Types.hpp"

namespace glu::ast {

/// @brief A memory arena that can be used to allocate memory for Types nodes.
class TypesMemoryArena : public MemoryArena {
public:
    TypesMemoryArena() = default;
    ~TypesMemoryArena() = default;

    /// @brief Allocate one AST node in the memory arena.
    /// @tparam T The type of the AST node to allocate.
    /// @tparam Args The types of the arguments to pass to the constructor of
    /// the AST node.
    /// @param args The arguments to pass to the constructor of the AST node.
    /// @return A pointer to the allocated AST node.
    template <typename T, typename... Args> T *createNode(Args &&...args)
    {
        static_assert(
            std::is_base_of_v<types::TypeBase, T>,
            "T must be a subclass of ASTNode"
        );

        return allocate<T>(std::forward<Args>(args)...);
    }
};

}

#endif /* GLU_AST_TYPESMEMORYARENA_HPP */
