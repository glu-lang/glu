#ifndef GLU_AST_DECL_FORBINDINGDECL_HPP
#define GLU_AST_DECL_FORBINDINGDECL_HPP

#include "VarLetDecl.hpp"

namespace glu::ast {

/// @class ForBindingDecl
/// @brief Represents a for-loop binding declaration.
///
/// This class is derived from VarLetDecl and represents a binding declaration
/// within a for-loop in the GLU AST (Abstract Syntax Tree).
class ForBindingDecl : public VarLetDecl {

public:
    /// @brief Constructs a ForBindingDecl object.
    ///
    /// @param location The source location of the declaration.
    /// @param name The name of the binding.
    /// @param type The type of the binding.
    ForBindingDecl(
        SourceLocation location, std::string name, glu::types::TypeBase *type
    )
        : VarLetDecl(
              NodeKind::ForBindingDeclKind, location, name, type, nullptr
          )
    {
    }

    /// @brief Checks if the given AST node is of type ForBindingDecl.
    ///
    /// @param node The AST node to check.
    /// @return True if the node is of type ForBindingDecl, false otherwise.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ForBindingDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FORBINDINGDECL_HPP
