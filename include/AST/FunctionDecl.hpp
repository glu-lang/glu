#ifndef GLU_AST_DECL_FUNCTIONDECL_HPP
#define GLU_AST_DECL_FUNCTIONDECL_HPP

#include "ASTNode.hpp"
#include "Param.hpp"
#include "Types/Types.hpp"
#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class FunctionDecl
/// @brief Represents a function declaration in the AST.
class FunctionDecl : public DeclBase {
    std::string _name;
    glu::types::FunctionTy *_type;
    llvm::SmallVector<Param> _params;
    glu::ast::ASTNode *_body;

public:
    /// @brief Constructor for the FunctionDecl class.
    /// @param location The source location of the function declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the function.
    /// @param type The type of the function.
    /// @param params A vector of Param objects representing the parameters of
    /// the function.
    /// @param body The body of the function.
    FunctionDecl(
        SourceLocation location, ASTNode *parent, std::string name,
        glu::types::FunctionTy *type, llvm::SmallVector<Param> params,
        ASTNode *body
    )
        : DeclBase(NodeKind::FunctionDeclKind, location, parent)
        , _name(std::move(name))
        , _type(type)
        , _params(std::move(params))
        , _body(body)
    {
    }

    /// @brief Getter for the name of the function.
    /// @return Returns the name of the function.
    std::string getName() const { return _name; }

    /// @brief Getter for the type of the function.
    /// @return Returns the type of the function.
    glu::types::FunctionTy *getType() const { return _type; }

    /// @brief Getter for the parameters of the function.
    /// @return Returns a vector of Param objects representing the parameters of
    /// the function.
    llvm::SmallVector<Param> const &getParams() const { return _params; }

    /// @brief Getter for the body of the function.
    /// @return Returns the body of the function.
    ASTNode *getBody() const { return _body; }

    /// @brief Static method to check if a node is a FunctionDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::FunctionDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FUNCTIONDECL_HPP
