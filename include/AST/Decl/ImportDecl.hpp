#ifndef GLU_AST_DECL_IMPORTDECL_HPP
#define GLU_AST_DECL_IMPORTDECL_HPP

#include "ASTNode.hpp"
#include "Types.hpp"
#include "llvm/Support/TrailingObjects.h"
#include <llvm/Support/Allocator.h>

namespace glu::ast {

/// @struct ImportPath
/// @brief Represents the import path as components.
///
/// The following examples show how import paths are decomposed:
///
/// - For "std::io::{println, eprint}":
///    - components: ["std", "io"]
///    - selectors: ["println", "eprint"]
///
/// - For "std::*":
///    - components: ["std"]
///    - selectors: ["*"]
///
/// - For "std":
///    - components: []
///    - selectors: ["std"]
///
/// - For "std::io":
///    - components: ["std"]
///    - selectors: ["io"]
///
/// - For "std::io::println":
///    - components: ["std", "io"]
///    - selectors: ["println"]
struct ImportPath {
    llvm::ArrayRef<llvm::StringRef> components;
    llvm::ArrayRef<llvm::StringRef> selectors;
};

/// @class ImportDecl
/// @brief Represents an import declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of an import
/// declaration.
class ImportDecl final
    : public DeclBase,
      private llvm::TrailingObjects<ImportDecl, llvm::StringRef> {
    friend llvm::TrailingObjects<ImportDecl, llvm::StringRef>;
    unsigned _numComponents;
    unsigned _numSelectors;

private:
    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename llvm::TrailingObjects<ImportDecl, llvm::StringRef>::OverloadToken<llvm::StringRef>) const
    {
        return _numComponents + _numSelectors;
    }

    ImportDecl(
        SourceLocation location, ASTNode *parent, ImportPath const &importPath
    )
        : DeclBase(NodeKind::ImportDeclKind, location, parent)
        , _numComponents(importPath.components.size())
        , _numSelectors(importPath.selectors.size())
    {
    }

public:
    static ImportDecl *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location, ASTNode *parent,
        ImportPath const &importPath
    )
    {
        auto size = sizeof(ImportDecl)
            + sizeof(llvm::StringRef)
                * (importPath.components.size() + importPath.selectors.size());
        void *memory = alloc.Allocate(size, alignof(ImportDecl));
        ImportDecl *importDecl
            = new (memory) ImportDecl(location, parent, importPath);
        llvm::StringRef *trailing
            = importDecl->getTrailingObjects<llvm::StringRef>();

        for (unsigned i = 0; i < importPath.components.size(); ++i) {
            new (&trailing[i]) llvm::StringRef(importPath.components[i]);
        }
        for (unsigned i = 0; i < importPath.selectors.size(); ++i) {
            new (&trailing[importPath.components.size() + i])
                llvm::StringRef(importPath.selectors[i]);
        }
        return importDecl;
    }

    ImportPath getImportPath() const
    {
        llvm::StringRef const *trailing
            = this->template getTrailingObjects<llvm::StringRef>();
        return ImportPath {
            llvm::ArrayRef<llvm::StringRef>(trailing, _numComponents),
            llvm::ArrayRef<llvm::StringRef>(
                trailing + _numComponents, _numSelectors
            )
        };
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ImportDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_IMPORTDECL_HPP
