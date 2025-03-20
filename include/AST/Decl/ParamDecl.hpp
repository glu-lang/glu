#ifndef GLU_AST_DECL_PARAMDECL_HPP
#define GLU_AST_DECL_PARAMDECL_HPP

#include "Decl/VarLetDecl.hpp"

namespace glu::ast {

/// @class ParamDecl
/// @brief Represents a parameter in a function declaration.
///
/// This class inherits from VarLetDecl and encapsulates the details of a
/// function parameters declaration.
class ParamDecl : public VarLetDecl {
public:
    /// @brief Constructor for the ParamDecl class.
    /// @param location The source location of the declaration.
    /// @param name The name of the declared param.
    /// @param type The type of the declared param.
    /// @param value The value assigned to the declared param.
    ParamDecl(
        SourceLocation location, llvm::StringRef name,
        glu::types::TypeBase *type, ExprBase *value
    )
        : VarLetDecl(NodeKind::ParamDeclKind, location, name, type, value)
    {
    }
    ParamDecl()
        : VarLetDecl(
              NodeKind::ParamDeclKind, SourceLocation(0), "", nullptr, nullptr
          )
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ParamDeclKind;
    }
};

}

#endif // GLU_AST_DECL_PARAMDECL_HPP
