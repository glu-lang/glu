#ifndef GLU_AST_DECL_LETDECL_HPP
#define GLU_AST_DECL_LETDECL_HPP

#include "ASTContext.hpp"
#include "Decl/VarLetDecl.hpp"

namespace glu::ast {

/// @class LetDecl
/// @brief Represents a let declaration in the AST.
///
/// This class inherits from VarLetDecl and encapsulates the details of a 'let'
/// declaration.
class LetDecl : public VarLetDecl {
public:
    /// @brief Constructor for the LetDecl class.
    /// @param location The source location of the declaration.
    /// @param name The name of the declared variable.
    /// @param type The type of the declared variable.
    /// @param value The value assigned to the declared variable.
    LetDecl(
        SourceLocation location, std::string name, glu::types::TypeBase *type,
        ExprBase *value
    )
        : VarLetDecl(
              NodeKind::LetDeclKind, location, std::move(name), type, value
          )
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::LetDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_LETDECL_HPP
