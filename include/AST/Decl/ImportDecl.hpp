#ifndef GLU_AST_DECL_IMPORTDECL_HPP
#define GLU_AST_DECL_IMPORTDECL_HPP

#include "ASTNode.hpp"
#include "Decl/DeclBase.hpp"
#include "Types.hpp"
#include "llvm/Support/TrailingObjects.h"
#include <llvm/Support/Allocator.h>

namespace glu::ast {

/// @struct ImportSelector
/// @brief Represents a selector with an optional alias in an import
/// declaration.
struct ImportSelector {
    llvm::StringRef name;
    llvm::StringRef alias; // Empty if no alias

    ImportSelector() : name(), alias() { }
    ImportSelector(llvm::StringRef n, llvm::StringRef a = "")
        : name(n), alias(a)
    {
    }

    /// @brief Gets the effective name (alias if present, otherwise the original
    /// name)
    llvm::StringRef getEffectiveName() const
    {
        return alias.empty() ? name : alias;
    }
};

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
///    - selectors: ["@all"]
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
    llvm::ArrayRef<ImportSelector> selectors;

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
                result += selector.name.str();
                if (!selector.alias.empty()) {
                    result += " as " + selector.alias.str();
                }
                result += ", ";
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
class ImportDecl final : public DeclBase,
                         private llvm::TrailingObjects<
                             ImportDecl, llvm::StringRef, ImportSelector> {
    friend llvm::TrailingObjects<ImportDecl, llvm::StringRef, ImportSelector>;
    unsigned _numComponents;
    unsigned _numSelectors;

private:
    // Methods required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(
        typename llvm::TrailingObjects<
            ImportDecl, llvm::StringRef,
            ImportSelector>::OverloadToken<llvm::StringRef>
    ) const
    {
        return _numComponents;
    }

    size_t numTrailingObjects(
        typename llvm::TrailingObjects<
            ImportDecl, llvm::StringRef,
            ImportSelector>::OverloadToken<ImportSelector>
    ) const
    {
        return _numSelectors;
    }

    ImportDecl(
        SourceLocation location, ASTNode *parent, ImportPath const &importPath,
        Visibility visibility, AttributeList *attributes
    )
        : DeclBase(
              NodeKind::ImportDeclKind, location, parent, visibility, attributes
          )
        , _numComponents(importPath.components.size())
        , _numSelectors(importPath.selectors.size())
    {
    }

public:
    static ImportDecl *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location, ASTNode *parent,
        ImportPath const &importPath, Visibility visibility,
        AttributeList *attributes
    )
    {
        auto size = totalSizeToAlloc<llvm::StringRef, ImportSelector>(
            importPath.components.size(), importPath.selectors.size()
        );
        void *memory = alloc.Allocate(size, alignof(ImportDecl));
        ImportDecl *importDecl = new (memory)
            ImportDecl(location, parent, importPath, visibility, attributes);
        std::uninitialized_copy(
            importPath.components.begin(), importPath.components.end(),
            importDecl->template getTrailingObjects<llvm::StringRef>()
        );
        std::uninitialized_copy(
            importPath.selectors.begin(), importPath.selectors.end(),
            importDecl->template getTrailingObjects<ImportSelector>()
        );
        return importDecl;
    }

    ImportPath getImportPath() const
    {
        llvm::StringRef const *components
            = getTrailingObjects<llvm::StringRef>();
        ImportSelector const *selectors = getTrailingObjects<ImportSelector>();
        return ImportPath {
            llvm::ArrayRef<llvm::StringRef>(components, _numComponents),
            llvm::ArrayRef<ImportSelector>(selectors, _numSelectors)
        };
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ImportDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_IMPORTDECL_HPP
