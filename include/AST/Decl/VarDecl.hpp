#ifndef GLU_AST_DECL_VARDECL_HPP
#define GLU_AST_DECL_VARDECL_HPP

#include "ASTNode.hpp"
#include "Types/Types.hpp"

#include <string>

namespace glu::ast {

/// @class VarDecl
/// @brief Represents a var declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a 'var'
/// declaration, including its name, type, and value.
class VarDecl : public DeclBase {
    std::string _name;
    glu::types::TypeBase *_type;
    glu::ast::ExprBase *_value;

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
        : DeclBase(NodeKind::VarDeclKind, location, nullptr)
        , _name(std::move(name))
        , _type(type)
        , _value(value)
    {
        _value->setParent(this);
    }

    /// @brief Getter for the name of the declared variable.
    /// @return Returns the name of the declared variable.
    std::string const &getName() const { return _name; }

    /// @brief Getter for the type of the declared variable.
    /// @return Returns the type of the declared variable.
    glu::types::TypeBase *getType() const { return _type; }

    /// @brief Getter for the value assigned to the declared variable.
    /// @return Returns the value assigned to the declared variable.
    glu::ast::ASTNode *getValue() const { return _value; }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_VARDECL_HPP
