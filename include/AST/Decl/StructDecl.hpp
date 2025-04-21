#ifndef GLU_AST_DECL_STRUCTDECL_HPP
#define GLU_AST_DECL_STRUCTDECL_HPP

#include "ASTContext.hpp"
#include "TypeDecl.hpp"
#include "Types.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class StructDecl
/// @brief Represents a struct declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a struct
/// declaration.
class StructDecl : public TypeDecl {
    using StructTy = glu::types::StructTy;
    using Field = glu::types::Field;
    StructTy *_self;

public:
    /// @brief Constructor for the StructDecl class.
    /// @param context The AST context.
    /// @param location The source location of the struct declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the struct.
    /// @param params A vector of Field objects representing the fields of
    /// the struct.
    StructDecl(
        ASTContext &context, SourceLocation location, ASTNode *parent,
        llvm::StringRef name, llvm::SmallVector<Field> fields
    )
        : TypeDecl(NodeKind::StructDeclKind, location, parent)
        , _self(context.getTypesMemoryArena().create<StructTy>(
              name, fields, location
          ))
    {
    }

    /// @brief Getter for the name of the struct.
    /// @return Returns the name of the struct.
    llvm::StringRef getName() const { return _self->getName(); }

    /// @brief Getter for the type of this struct.
    /// @return Returns the type of this struct.
    StructTy *getType() const { return _self; }

    /// @brief Static method to check if a node is a StructDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::StructDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_STRUCTDECL_HPP
