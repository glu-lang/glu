#ifndef GLU_AST_DECL_TYPEDECL_HPP
#define GLU_AST_DECL_TYPEDECL_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

namespace glu::ast {

/// @class TypeDecl
/// @brief Represents a common base class for type declarations in the
/// AST.
///
/// This class inherits from DeclBase and encapsulates the common details of a
/// type declaration, including its name and corresponding type.
class TypeDecl : public DeclBase {
public:
    TypeDecl(
        NodeKind kind, SourceLocation location, ASTNode *parent,
        Visibility visibility = Visibility::Private
    )
        : DeclBase(kind, location, parent, visibility)
    {
    }

    /// Getter for the name of the declared type
    llvm::StringRef getName() const;

    /// Getter for the declared type
    glu::types::TypeBase *getType() const;

    static bool classof(ASTNode const *node)
    {
        return node->getKind() > NodeKind::TypeDeclFirstKind
            && node->getKind() < NodeKind::TypeDeclLastKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_TYPEDECL_HPP
