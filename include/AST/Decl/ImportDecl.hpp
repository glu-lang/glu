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

    std::string toString() const
    {
        std::string result;

        for (auto &component : components) {
            result += component.str() + "::";
        }

        if (!selectors.empty()) {
            if (!components.empty()) {
                result += "{";
            }
            for (auto &selector : selectors) {
                result += selector.str() + ", ";
            }
            result.pop_back();
            result.pop_back();
            if (!components.empty()) {
                result += "}";
            }
        }
        return result;
    }
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
    size_t numTrailingObjects(
        typename llvm::TrailingObjects<
            ImportDecl, llvm::StringRef>::OverloadToken<llvm::StringRef>
    ) const
    {
        return _numComponents + _numSelectors;
    }

    ImportDecl(
        SourceLocation location, ASTNode *parent, ImportPath const &importPath
    )
        : DeclBase(NodeKind::ImportDeclKind, location, parent, Visibility::Private)
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
        auto size = totalSizeToAlloc<llvm::StringRef>(
            importPath.components.size() + importPath.selectors.size()
        );
        void *memory = alloc.Allocate(size, alignof(ImportDecl));
        ImportDecl *importDecl
            = new (memory) ImportDecl(location, parent, importPath);
        std::uninitialized_copy(
            importPath.components.begin(), importPath.components.end(),
            importDecl->template getTrailingObjects<llvm::StringRef>()
        );
        std::uninitialized_copy(
            importPath.selectors.begin(), importPath.selectors.end(),
            importDecl->template getTrailingObjects<llvm::StringRef>()
                + importPath.components.size()
        );
        return importDecl;
    }

    ImportPath getImportPath() const
    {
        llvm::StringRef const *trailing = getTrailingObjects<llvm::StringRef>();
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
