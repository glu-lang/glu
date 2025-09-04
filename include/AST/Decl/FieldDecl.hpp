#ifndef GLU_AST_DECL_FIELDDECL_HPP
#define GLU_AST_DECL_FIELDDECL_HPP

#include "Decl/VarLetDecl.hpp"

namespace glu::ast {

/// @class FieldDecl
/// @brief Represents a field declaration within a struct.
///
/// This class inherits from VarLetDecl and encapsulates the details of a field
/// declaration within a struct in the GLU AST (Abstract Syntax Tree).
class FieldDecl : public VarLetDecl {
public:
    /// @brief Constructor for the FieldDecl class.
    /// @param location The source location of the field declaration.
    /// @param name The name of the field.
    /// @param type The type of the field.
    /// @param value The default value for the field (optional, can be nullptr).
    FieldDecl(
        SourceLocation location, llvm::StringRef name,
        glu::types::TypeBase *type, ExprBase *value = nullptr
    )
        : VarLetDecl(NodeKind::FieldDeclKind, location, name, type, value)
    {
    }

    /// @brief Static method to check if a node is a FieldDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::FieldDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FIELDDECL_HPP
