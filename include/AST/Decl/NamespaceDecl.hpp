#ifndef GLU_AST_DECL_NAMESPACEDECL_HPP
#define GLU_AST_DECL_NAMESPACEDECL_HPP

#include "ASTNodeMacros.hpp"
#include "Decl/DeclBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @class NamespaceDecl
/// @brief Represents a namespace declaration in the AST.
///
/// Namespaces behave similarly to modules but are always nested within a
/// ModuleDecl or another NamespaceDecl, allowing Glu code to merge declarations
/// under a shared qualified name.
class NamespaceDecl final
    : public DeclBase,
      private llvm::TrailingObjects<NamespaceDecl, DeclBase *> {
    llvm::StringRef _name;

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        NamespaceDecl, _numDecls, DeclBase *, Decls
    )

    NamespaceDecl(
        SourceLocation location, ASTNode *parent, llvm::StringRef name,
        llvm::ArrayRef<DeclBase *> decls, Visibility visibility,
        AttributeList *attributes
    )
        : DeclBase(
              NodeKind::NamespaceDeclKind, location, parent, visibility,
              attributes
          )
        , _name(name)
    {
        initDecls(decls);
    }

public:
    /// @brief Create a new NamespaceDecl instance.
    static NamespaceDecl *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location, ASTNode *parent,
        llvm::StringRef name, llvm::ArrayRef<DeclBase *> decls,
        Visibility visibility = Visibility::Public,
        AttributeList *attributes = nullptr
    )
    {
        void *mem = alloc.Allocate(
            totalSizeToAlloc<DeclBase *>(decls.size()), alignof(NamespaceDecl)
        );

        return new (mem) NamespaceDecl(
            location, parent, name, decls, visibility, attributes
        );
    }

    /// @brief Get the simple name of this namespace.
    llvm::StringRef getName() const { return _name; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::NamespaceDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_NAMESPACEDECL_HPP
