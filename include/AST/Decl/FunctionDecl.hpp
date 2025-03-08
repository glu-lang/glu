#ifndef GLU_AST_DECL_FUNCTIONDECL_HPP
#define GLU_AST_DECL_FUNCTIONDECL_HPP

#include "ASTNode.hpp"
#include "Decl/ParamDecl.hpp"
#include "Stmt/CompoundStmt.hpp"
#include "Types.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class FunctionDecl
/// @brief Represents a function declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a function
/// declaration, including its name, type, parameters, and body.
class FunctionDecl final
    : public DeclBase,
      private llvm::TrailingObjects<FunctionDecl, ParamDecl> {
public:
    friend llvm::TrailingObjects<FunctionDecl, ParamDecl>;

private:
    using TrailingParamas = llvm::TrailingObjects<FunctionDecl, ParamDecl>;
    std::string _name;
    glu::types::FunctionTy *_type;
    CompoundStmt _body;

    unsigned _numParams;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(typename TrailingParams::OverloadToken<ParamDecl>)
        const
    {
        return _numParams;
    }

    FunctionDecl(
        SourceLocation location, ASTNode *parent, std::string name,
        glu::types::FunctionTy *type, llvm::ArrayRef<ParamDecl> const params
    )
        : DeclBase(NodeKind::FunctionDeclKind, location, parent)
        , _name(std::move(name))
        , _type(type)
        , _body(CompoundStmt(location, {}))
        , _numParams(params.size())
    {
        std::uninitialized_copy(
            params.begin(), params.end(),
            this->template getTrailingObjects<ParamDecl>()
        );
        _body.setParent(this);
    }

public:
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
        std::string name, glu::types::FunctionTy *type,
        llvm::ArrayRef<ParamDecl> const params
    )
    {
        auto totalSize = totalSizeToAlloc<ParamDecl>(params.size());
        void *mem = alloc.Allocate(totalSize, alignof(FunctionDecl));
        FunctionDecl *decl = new (mem)
            FunctionDecl(location, parent, std::move(name), type, params);

        return decl;
    }

    /// @brief Getter for the name of the function.
    /// @return Returns the name of the function.
    std::string getName() const { return _name; }

    /// @brief Getter for the type of the function.
    /// @return Returns the type of the function.
    glu::types::FunctionTy *getType() const { return _type; }

    /// @brief Getter for the parameters of the function.
    /// @param index The index of the parameter to retrieve.
    /// @return Returns the parameter at the given index.
    ParamDecl const &getParam(size_t index) const
    {
        assert(index < _numParams && "Index out of bounds");
        return this->template getTrailingObjects<ParamDecl>()[index];
    }

    /// @brief Getter for the number of parameters of the function.
    /// @return Returns the number of parameters of the function.
    unsigned getParamsCount() const { return _numParams; }

    /// @brief Getter of the index of a parameter.
    /// @param name The name of the parameter to retrieve.
    /// @return Returns the asked parameter index.
    std::optional<size_t> getParamIndex(llvm::StringRef name) const
    {
        for (size_t i = 0; i < _numParams; ++i) {
            if (getParam(i).getName() == name) {
                return i;
            }
        }

        return std::nullopt;
    }

    /// @brief Getter for the number of parameters of the function.
    /// @return Returns the number of parameters of the function.
    size_t getParamCount() const { return _numParams; }

    /// @brief Getter for the body of the function.
    /// @return Returns the body of the function.
    CompoundStmt *getBody() { return &_body; }

    /// @brief Static method to check if a node is a FunctionDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::FunctionDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FUNCTIONDECL_HPP
