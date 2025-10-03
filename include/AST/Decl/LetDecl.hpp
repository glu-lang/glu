#ifndef GLU_AST_DECL_LETDECL_HPP
#define GLU_AST_DECL_LETDECL_HPP

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
    /// @param visibility The visibility of the variable.
    LetDecl(
        SourceLocation location, llvm::StringRef name,
        glu::types::TypeBase *type, ExprBase *value,
        AttributeList *attributes = nullptr,
        Visibility visibility = Visibility::Private
    )
        : VarLetDecl(
              NodeKind::LetDeclKind, location, name, type, value, visibility,
              attributes
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
