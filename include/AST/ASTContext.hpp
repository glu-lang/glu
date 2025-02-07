#ifndef GLU_AST_ASTCONTEXT_HPP
#define GLU_AST_ASTCONTEXT_HPP

#include "ASTMemoryArena.hpp"

namespace glu::ast {

class ASTContext {
    ASTMemoryArena _arena;

public:
    ASTContext() = default;

    /// @brief Get the memory arena used by the AST context.
    /// @return The memory arena used by the AST context.
    ASTMemoryArena &getMemoryArena() { return _arena; }
};

}; // namespace glu::ast

#endif /* GLU_AST_ASTCONTEXT_HPP */
