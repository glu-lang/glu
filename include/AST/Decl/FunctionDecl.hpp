#ifndef GLU_AST_DECL_FUNCTIONDECL_HPP
#define GLU_AST_DECL_FUNCTIONDECL_HPP

#include "ASTNode.hpp"
#include "Types/FunctionTy.hpp"

namespace glu::ast {

/// @class FunctionDecl
/// @brief Represents a function declaration in the AST.
class FunctionDecl : public DeclBase {
    std::string _name;
    glu::types::FunctionTy *_type;

public:
    /// @brief Constructor for the FunctionDecl class.
    /// @param name The name of the function.
    /// @param type The type of the function.
    /// @param location The source location of the function declaration.
    /// @param parent The parent AST node.
    FunctionDecl(
        std::string name, glu::types::FunctionTy *type, 
        SourceLocation location, ASTNode *parent = nullptr
    )
        : DeclBase(NodeKind::FunctionDeclKind, location, parent)
        , _name(std::move(name))
        , _type(type)
    {
    }

    /// @brief Getter for the name of the function.
    /// @return Returns the name of the function.
    std::string getName() const { return _name; }

    /// @brief Getter for the type of the function.
    /// @return Returns the type of the function.
    glu::types::FunctionTy *getType() const { return _type; }

    /// @brief Static method to check if a node is a FunctionDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::FunctionDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FUNCTIONDECL_HPP
