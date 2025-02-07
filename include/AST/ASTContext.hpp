#ifndef GLU_AST_ASTCONTEXT_HPP
#define GLU_AST_ASTCONTEXT_HPP

#include "ASTMemoryArena.hpp"
#include "TypesMemoryArena.hpp"

namespace glu::ast {

class ASTContext {
    ASTMemoryArena _astMemoryArena;
    TypesMemoryArena _typesMemoryArena;

public:
    ASTContext() = default;

    /// @brief Get the memory arena used by the AST context.
    /// @return The memory arena used by the AST context.
    ASTMemoryArena &getASTMemoryArena() { return _astMemoryArena; }

    /// @brief Get the memory arena used by the types in the AST context.
    /// @return The memory arena used by the types in the AST context.
    TypesMemoryArena &getTypesMemoryArena() { return _typesMemoryArena; }
};

}; // namespace glu::ast

#endif /* GLU_AST_ASTCONTEXT_HPP */
