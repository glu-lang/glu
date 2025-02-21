#ifndef GLU_AST_DECL_ENUMDECL_HPP
#define GLU_AST_DECL_ENUMDECL_HPP

#include "ASTContext.hpp"
#include "ASTNode.hpp"
#include "Types.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class EnumDecl
/// @brief Represents a enum declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a enum
/// declaration.
class EnumDecl : public DeclBase {
    using EnumTy = glu::types::EnumTy;
    using Case = glu::types::Case;
    EnumTy *_self;

public:
    /// @brief Constructor for the EnumDecl class.
    /// @param context The AST context.
    /// @param location The source location of the enum declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the enum.
    /// @param params A vector of Case objects representing the cases of
    /// the enum.
    EnumDecl(
        ASTContext &context, SourceLocation location, ASTNode *parent,
        std::string name, llvm::SmallVector<Case> cases
    )
        : DeclBase(NodeKind::EnumDeclKind, location, parent)
        , _self(context.getTypesMemoryArena().create<EnumTy>(
              name, cases, location
          ))
    {
    }

    /// @brief Getter for the name of the enum.
    /// @return Returns the name of the enum.
    llvm::StringRef getName() const { return _self->getName(); }

    /// @brief Getter for the type of this enum.
    /// @return Returns the type of this enum.
    EnumTy *getType() const { return _self; }

    /// @brief Static method to check if a node is a EnumDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::EnumDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_ENUMDECL_HPP
