#ifndef GLU_AST_DECL_LETNDECL_HPP
#define GLU_AST_DECL_LETNDECL_HPP

#include "ASTNode.hpp"
#include "Types/Types.hpp"

#include <string>

namespace glu::ast {

/// @class LetDecl
/// @brief Represents a let declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a 'let'
/// declaration, including its name, type, and value.
class LetDecl : public DeclBase {
    std::string _name;
    glu::types::TypeBase *_type;
    glu::ast::ASTNode *_value;

public:
    /// @brief Constructor for the LetDecl class.
    /// @param location The source location of the declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the declared variable.
    /// @param type The type of the declared variable.
    /// @param value The value assigned to the declared variable.
    LetDecl(
        SourceLocation location, ASTNode *parent, std::string name,
        glu::types::TypeBase *type, ASTNode *value
    )
        : DeclBase(NodeKind::LetDeclKind, location, parent)
        , _name(std::move(name))
        , _type(type)
        , _value(value)
    {
    }

    /// @brief Getter for the name of the declared variable.
    /// @return Returns the name of the declared variable.
    std::string getName() const { return _name; }

    /// @brief Getter for the type of the declared variable.
    /// @return Returns the type of the declared variable.
    glu::types::TypeBase *getType() const { return _type; }

    /// @brief Getter for the value assigned to the declared variable.
    /// @return Returns the value assigned to the declared variable.
    glu::ast::ASTNode *getValue() const { return _value; }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_LETNDECL_HPP
