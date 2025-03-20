#ifndef GLU_AST_DECL_FUNCTIONDECL_HPP
#define GLU_AST_DECL_FUNCTIONDECL_HPP

#include "ASTNode.hpp"
#include "Decl/ParamDecl.hpp"
#include "Stmt/CompoundStmt.hpp"
#include "Types.hpp"

#include <llvm/ADT/ArrayRef.h>

namespace glu::ast {

/// @class FunctionDecl
/// @brief Represents a function declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a function
/// declaration, including its name, type, parameters, and body.
class FunctionDecl final
    : public DeclBase,
      private llvm::TrailingObjects<FunctionDecl, ParamDecl *> {
private:
    using TrailingParams = llvm::TrailingObjects<FunctionDecl, ParamDecl *>;
    llvm::StringRef _name;
    glu::types::FunctionTy *_type;
    CompoundStmt *_body;

    unsigned _numParams;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename TrailingParams::OverloadToken<ParamDecl *>)
            const
    {
        return _numParams;
    }

    FunctionDecl(
        SourceLocation location, ASTNode *parent, llvm::StringRef name,
        glu::types::FunctionTy *type, llvm::ArrayRef<ParamDecl *> params,
        CompoundStmt *body
    )
        : DeclBase(NodeKind::FunctionDeclKind, location, parent)
        , _name(std::move(name))
        , _type(type)
        , _body(body)
        , _numParams(params.size())
    {
        _body->setParent(this);
        std::uninitialized_copy(
            params.begin(), params.end(), getTrailingObjects<ParamDecl *>()
        );
        for (unsigned i = 0; i < _numParams; i++) {
            getTrailingObjects<ParamDecl *>()[i]->setParent(this);
        }
    }

public:
    friend TrailingParams;

    /// @brief Static method to create a new FunctionDecl.
    /// @param alloc The allocator used to create the FunctionDecl.
    /// @param location The source location of the function declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the function.
    /// @param type The type of the function.
    /// @param params A vector of Param objects representing the parameters of
    /// @return Returns a new FunctionDecl.
    static FunctionDecl *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location, ASTNode *parent,
        llvm::StringRef name, glu::types::FunctionTy *type,
        llvm::ArrayRef<ParamDecl *> params, CompoundStmt *body
    )
    {
        auto totalSize = totalSizeToAlloc<ParamDecl *>(params.size());
        void *mem = alloc.Allocate(totalSize, alignof(FunctionDecl));
        return new (mem)
            FunctionDecl(location, parent, name, type, params, body);
    }

    /// @brief Getter for the name of the function.
    /// @return Returns the name of the function.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the type of the function.
    /// @return Returns the type of the function.
    glu::types::FunctionTy *getType() const { return _type; }

    /// @brief Getter for the parameters of the function.
    /// @return Returns the parameters.
    llvm::ArrayRef<ParamDecl *> getParams() const
    {
        return { this->template getTrailingObjects<ParamDecl *>(), _numParams };
    }

    /// @brief Getter of the index of a parameter.
    /// @param name The name of the parameter to retrieve.
    /// @return Returns the asked parameter index.
    std::optional<size_t> getParamIndex(llvm::StringRef name) const
    {
        auto params = this->getParams();

        auto searchedParam = std::find_if(
            params.begin(), params.end(),
            [name](ParamDecl const *param) { return param->getName() == name; }
        );

        if (searchedParam == params.end())
            return std::nullopt;
        return std::distance(params.begin(), searchedParam);
    }

    /// @brief Getter for the number of parameters of the function.
    /// @return Returns the number of parameters of the function.
    size_t getParamCount() const { return _numParams; }

    /// @brief Getter for the body of the function.
    /// @return Returns the body of the function.
    CompoundStmt *getBody() { return _body; }

    /// @brief Static method to check if a node is a FunctionDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::FunctionDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FUNCTIONDECL_HPP
