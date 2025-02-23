#ifndef GLU_AST_DECL_TYPEALIASDECL_HPP
#define GLU_AST_DECL_TYPEALIASDECL_HPP

#include "ASTContext.hpp"
#include "ASTNode.hpp"
#include "Types.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class TypeAliasDecl
/// @brief Represents a type alias declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a type
/// alias declaration.
class TypeAliasDecl : public DeclBase {
    using TypeAliasTy = glu::types::TypeAliasTy;
    TypeAliasTy *_self;

public:
    /// @brief Constructor for the TypeAliasDecl class.
    /// @param context The AST context.
    /// @param location The source location of the type alias declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the type alias.
    /// @param params A vector of Case objects representing the cases of
    /// the type alias.
    TypeAliasDecl(
        ASTContext &context, SourceLocation location, ASTNode *parent,
        std::string name, glu::types::TypeBase *wrapped
    )
        : DeclBase(NodeKind::TypeAliasDeclKind, location, parent)
        , _self(context.getTypesMemoryArena().create<TypeAliasTy>(
              wrapped, name, location
          ))
    {
    }

    /// @brief Getter for the name of the type alias.
    /// @return Returns the name of the type alias.
    std::string getName() const { return _self->getName(); }

    /// @brief Getter for the type of this type alias.
    /// @return Returns the type of this type alias.
    TypeAliasTy *getType() const { return _self; }

    /// @brief Static method to check if a node is a TypeAliasDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::TypeAliasDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_TYPEALIASDECL_HPP
