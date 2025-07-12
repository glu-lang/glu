#ifndef GLU_AST_DECL_VARLETDECL_HPP
#define GLU_AST_DECL_VARLETDECL_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

namespace glu::ast {

/// @class VarLetDecl
/// @brief Represents a common base class for var and let declarations in the
/// AST.
///
/// This class inherits from DeclBase and encapsulates the common details of a
/// 'var' and 'let' declaration, including its name, type, and value.
class VarLetDecl : public DeclBase {
protected:
    llvm::StringRef _name;
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
        NodeKind kind, SourceLocation location, llvm::StringRef name,
        glu::types::TypeBase *type, ExprBase *value
    )
        : DeclBase(kind, location, nullptr)
        , _name(name)
        , _type(type)
        , _value(value)
    {
        if (_value)
            _value->setParent(this);
    }

    /// @brief Getter for the name of the declared variable.
    /// @return Returns the name of the declared variable.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the type of the declared variable.
    /// @return Returns the type of the declared variable.
    glu::types::TypeBase *getType() const { return _type; }

    /// @brief Getter for the value assigned to the declared variable.
    /// @return Returns the value assigned to the declared variable.
    glu::ast::ExprBase *getValue() const { return _value; }

    /// @brief Set the name of the declared variable.
    /// @param name The name to set.
    void setName(llvm::StringRef name) { _name = name; }

    /// @brief Set the type of the variable or constant.
    /// @param type The type to set.
    void setType(glu::types::TypeBase *type) { _type = type; }

    /// @brief Set the initial value expression of the variable or constant.
    /// @param value The value expression to set.
    void setValue(ExprBase *value)
    {
        _value = value;
        if (_value != nullptr) {
            _value->setParent(this);
        }
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() > NodeKind::VarLetDeclFirstKind
            && node->getKind() < NodeKind::VarLetDeclLastKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_VARLETDECL_HPP
