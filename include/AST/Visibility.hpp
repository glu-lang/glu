#ifndef GLU_AST_VISIBILITY_HPP
#define GLU_AST_VISIBILITY_HPP

namespace glu::ast {

/// @brief Represents the visibility of a declaration.
enum class Visibility {
    /// @brief The declaration is private (default).
    /// Private declarations are only visible within the same module.
    Private,

    /// @brief The declaration is public.
    /// Public declarations are visible to importing modules.
    Public
};

} // namespace glu::ast

#endif // GLU_AST_VISIBILITY_HPP
