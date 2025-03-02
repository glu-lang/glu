#ifndef GLU_AST_DECL_VARDECL_HPP
#define GLU_AST_DECL_VARDECL_HPP

#include "ASTContext.hpp"
#include "Decl/VarLetDecl.hpp"

namespace glu::ast {

/// @class VarDecl
/// @brief Represents a var declaration in the AST.
///
/// This class inherits from VarLetDecl and encapsulates the details of a 'var'
/// declaration.
class VarDecl : public VarLetDecl {
public:
    /// @brief Constructor for the VarDecl class.
    /// @param location The source location of the declaration.
    /// @param name The name of the declared variable.
    /// @param type The type of the declared variable.
    /// @param value The value assigned to the declared variable.
    VarDecl(
        SourceLocation location, std::string name, glu::types::TypeBase *type,
        ExprBase *value
    )
        : VarLetDecl(
              NodeKind::VarDeclKind, location, std::move(name), type, value
          )
    {
    }

    /// @brief Creates a VarDecl node and allocates it in the AST context's
    /// memory arena.
    /// @param context The AST context.
    /// @param location The source location of the declaration.
    /// @param name The name of the declared variable.
    /// @param type The type of the declared variable.
    /// @param value The value assigned to the declared variable.
    /// @return A pointer to the newly created VarDecl node.
    static VarDecl *create(
        ASTContext &context, SourceLocation location, std::string name,
        glu::types::TypeBase *type, ExprBase *value
    )
    {
        return context.getASTMemoryArena().create<VarDecl>(
            location, std::move(name), type, value
        );
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::VarDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_VARDECL_HPP
