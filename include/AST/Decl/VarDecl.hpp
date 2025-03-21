#ifndef GLU_AST_DECL_VARDECL_HPP
#define GLU_AST_DECL_VARDECL_HPP

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
        SourceLocation location, llvm::StringRef name,
        glu::types::TypeBase *type, ExprBase *value
    )
        : VarLetDecl(NodeKind::VarDeclKind, location, name, type, value)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::VarDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_VARDECL_HPP
