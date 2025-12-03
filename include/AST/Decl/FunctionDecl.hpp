#ifndef GLU_AST_DECL_FUNCTIONDECL_HPP
#define GLU_AST_DECL_FUNCTIONDECL_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"
#include "Attributes.hpp"
#include "Decl/DeclBase.hpp"
#include "Decl/ParamDecl.hpp"
#include "Decl/TemplateParameterDecl.hpp"
#include "Stmt/CompoundStmt.hpp"
#include "Types.hpp"

#include <llvm/ADT/ArrayRef.h>

namespace glu::ast {

/// @class FunctionDecl
/// @brief Represents a function declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a function
/// declaration, including its name, type, parameters, and body.

enum class BuiltinKind {
#define BUILTINS_KIND(ID) ID##Kind,
#include "Builtins.def"
    None
};

class FunctionDecl final
    : public DeclBase,
      private llvm::TrailingObjects<FunctionDecl, ParamDecl *> {
private:
    llvm::StringRef _name;
    glu::types::FunctionTy *_type;
    BuiltinKind _builtinKind = BuiltinKind::None;

    GLU_AST_GEN_CHILD(
        FunctionDecl, TemplateParameterList *, _templateParams, TemplateParams
    )
    GLU_AST_GEN_CHILD(FunctionDecl, CompoundStmt *, _body, Body)
    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        FunctionDecl, _numParams, ParamDecl *, Params
    )

    FunctionDecl(
        SourceLocation location, ASTNode *parent, llvm::StringRef name,
        glu::types::FunctionTy *type, llvm::ArrayRef<ParamDecl *> params,
        CompoundStmt *body, TemplateParameterList *templateParams,
        Visibility visibility = Visibility::Private,
        AttributeList *attributes = nullptr
    )
        : DeclBase(
              NodeKind::FunctionDeclKind, location, parent, visibility,
              attributes
          )
        , _name(std::move(name))
        , _type(type)
    {
        initTemplateParams(templateParams, /* nullable = */ true);
        initBody(body, /* nullable = */ true);
        initParams(params);
    }

    FunctionDecl(
        SourceLocation location, llvm::StringRef name,
        glu::types::FunctionTy *type, llvm::ArrayRef<ParamDecl *> params,
        BuiltinKind builtinKind, Visibility visibility = Visibility::Private
    )
        : DeclBase(
              NodeKind::FunctionDeclKind, location, nullptr, visibility, nullptr
          )
        , _name(std::move(name))
        , _type(type)
        , _builtinKind(builtinKind)
    {
        initTemplateParams(nullptr, /* nullable = */ true);
        initBody(nullptr, /* nullable = */ true);
        initParams(params);
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
        llvm::StringRef name, glu::types::FunctionTy *type,
        llvm::ArrayRef<ParamDecl *> params, CompoundStmt *body,
        TemplateParameterList *templateParams = nullptr,
        Visibility visibility = Visibility::Private,
        AttributeList *attributes = nullptr
    )
    {
        auto totalSize = totalSizeToAlloc<ParamDecl *>(params.size());
        void *mem = alloc.Allocate(totalSize, alignof(FunctionDecl));
        return new (mem) FunctionDecl(
            location, parent, name, type, params, body, templateParams,
            visibility, attributes
        );
    }

    /// @brief Static method to create a builtin FunctionDecl.
    /// @param alloc The allocator used to create the FunctionDecl.
    /// @param location The source location of the function declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the function.
    /// @param type The type of the function.
    /// @param params A vector of Param objects representing the parameters of
    /// @return Returns a new FunctionDecl.
    static FunctionDecl *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location,
        llvm::StringRef name, glu::types::FunctionTy *type,
        llvm::ArrayRef<ParamDecl *> params, BuiltinKind builtinKind,
        Visibility visibility = Visibility::Private
    )
    {
        auto totalSize = totalSizeToAlloc<ParamDecl *>(params.size());
        void *mem = alloc.Allocate(totalSize, alignof(FunctionDecl));
        return new (mem)
            FunctionDecl(location, name, type, params, builtinKind, visibility);
    }

    /// @brief Getter for the name of the function.
    /// @return Returns the name of the function.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the type of the function.
    /// @return Returns the type of the function.
    glu::types::FunctionTy *getType() const { return _type; }

    /// @brief Setter for the type of the function.
    /// @param type The type to set.
    void setType(glu::types::FunctionTy *type) { _type = type; }

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

    /// @brief Getter for the number of required parameters (those without
    /// default values).
    /// @return Returns the number of required parameters.
    size_t getRequiredParamCount() const
    {
        size_t requiredCount = 0;
        while (requiredCount < _numParams
               && getParams()[requiredCount]->getValue() == nullptr) {
            ++requiredCount;
        }
        return requiredCount;
    }

    /// @brief Static method to check if a node is a FunctionDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::FunctionDeclKind;
    }

    bool isBuiltin() const { return _builtinKind != BuiltinKind::None; }
    BuiltinKind getBuiltinKind() const { return _builtinKind; }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FUNCTIONDECL_HPP
