#ifndef GLU_AST_ASTCONTEXT_HPP
#define GLU_AST_ASTCONTEXT_HPP

#include "ASTNode.hpp"
#include "Basic/TypedMemoryArena.hpp"
#include "Types.hpp"

namespace glu::ast {

class ASTContext {
    TypedMemoryArena<ASTNode> _astMemoryArena;
    TypedMemoryArena<types::TypeBase> _typesMemoryArena;

public:
    ASTContext() = default;

    /// @brief Get the memory arena used by the AST context.
    /// @return The memory arena used by the AST context.
    TypedMemoryArena<ASTNode> &getASTMemoryArena() { return _astMemoryArena; }

    /// @brief Get the memory arena used by the types in the AST context.
    /// @return The memory arena used by the types in the AST context.
    TypedMemoryArena<types::TypeBase> &getTypesMemoryArena()
    {
        return _typesMemoryArena;
    }
};

}; // namespace glu::ast

#endif /* GLU_AST_ASTCONTEXT_HPP */
