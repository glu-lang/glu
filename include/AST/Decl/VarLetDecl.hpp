#ifndef GLU_AST_DECL_VARLETDECL_HPP
#define GLU_AST_DECL_VARLETDECL_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

#include <string>

namespace glu::ast {

/// @class VarLetDecl
/// @brief Represents a common base class for var and let declarations in the
/// AST.
///
/// This class inherits from DeclBase and encapsulates the common details of a
/// 'var' and 'let' declaration, including its name, type, and value.
class VarLetDecl : public DeclBase {
protected:
    std::string _name;
    glu::types::TypeBase *_type;
    glu::ast::ExprBase *_value;

public:
    /// @brief Constructor for the VarLetDecl class.
    /// @param kind The kind of the declaration (VarDeclKind or LetDeclKind).
    /// @param location The source location of the declaration.
    /// @param name The name of the declared variable.
    /// @param type The type of the declared variable.
    /// @param value The value assigned to the declared variable.
    VarLetDecl(
        NodeKind kind, SourceLocation location, std::string name,
        glu::types::TypeBase *type, ExprBase *value
    )
        : DeclBase(kind, location, nullptr)
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

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::VarDeclKind
            || node->getKind() == NodeKind::LetDeclKind
            || node->getKind() == NodeKind::ForBindingDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_VARLETDECL_HPP
