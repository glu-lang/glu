#ifndef GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP
#define GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP

#include "Basic/SourceLocation.hpp"
#include "Basic/Tokens.hpp"

#include "TypeBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @struct NamespaceIdentifier
/// @brief Represents an identifier with namespaces maybe.
///
/// The following examples show how identifiers are decomposed:
///
/// - For "std::io::eprint":
///    - components: ["std", "io"]
///    - identifier: "eprint"
///
/// - For "llvm::APInt":
///    - components: ["llvm"]
///    - identifier: "APInt"
///
/// - For "machin":
///    - components: []
///    - identifier: "machin"
struct NamespaceIdentifier {
    llvm::ArrayRef<llvm::StringRef> components;
    llvm::StringRef identifier;

    std::string toString()
    {
        std::string result;

        for (auto &component : components) {
            result += component.str() + "::";
        }

        result += identifier.str();
        return result;
    }

    static NamespaceIdentifier fromOp(Token t)
    {
        return NamespaceIdentifier { llvm::ArrayRef<llvm::StringRef>(),
                                     t.getLexeme() };
    }
};

} // namespace glu::ast

namespace glu::types {

/// @brief UnresolvedNameTy is a class that represents an unresolved name type
/// in the AST.
class UnresolvedNameTy final
    : public TypeBase,
      private llvm::TrailingObjects<
          UnresolvedNameTy, llvm::StringRef, glu::types::TypeBase *> {
    using TrailingArgs = llvm::TrailingObjects<
        UnresolvedNameTy, llvm::StringRef, glu::types::TypeBase *>;
    friend TrailingArgs;

    unsigned _numComponents;
    unsigned _numTemplateArgs;
    SourceLocation _location;

    size_t numTrailingObjects(
        typename TrailingArgs::OverloadToken<llvm::StringRef>
    ) const
    {
        return _numComponents + 1;
    }

    size_t numTrailingObjects(
        typename TrailingArgs::OverloadToken<glu::types::TypeBase *>
    ) const
    {
        return _numTemplateArgs;
    }

    UnresolvedNameTy(
        ast::NamespaceIdentifier const &name,
        llvm::ArrayRef<glu::types::TypeBase *> templateArgs,
        SourceLocation location
    )
        : TypeBase(TypeKind::UnresolvedNameTyKind)
        , _numComponents(name.components.size())
        , _numTemplateArgs(templateArgs.size())
        , _location(location)
    {
        std::uninitialized_copy(
            name.components.begin(), name.components.end(),
            getTrailingObjects<llvm::StringRef>()
        );
        std::memcpy(
            getTrailingObjects<llvm::StringRef>() + _numComponents,
            &name.identifier, sizeof(llvm::StringRef)
        );
        std::uninitialized_copy(
            templateArgs.begin(), templateArgs.end(),
            getTrailingObjects<glu::types::TypeBase *>()
        );
    }

public:
    static UnresolvedNameTy *create(
        llvm::BumpPtrAllocator &allocator, ast::NamespaceIdentifier const &name,
        llvm::ArrayRef<glu::types::TypeBase *> templateArgs,
        SourceLocation location
    )
    {
        auto totalSize
            = totalSizeToAlloc<llvm::StringRef, glu::types::TypeBase *>(
                name.components.size() + 1, templateArgs.size()
            );
        void *mem = allocator.Allocate(totalSize, alignof(UnresolvedNameTy));
        return new (mem) UnresolvedNameTy(name, templateArgs, location);
    }

    static UnresolvedNameTy *create(
        llvm::BumpPtrAllocator &allocator, ast::NamespaceIdentifier const &name,
        SourceLocation location
    )
    {
        return create(
            allocator, name, llvm::ArrayRef<glu::types::TypeBase *> {}, location
        );
    }
    /// @brief Getter for the name of the unresolved type.
    /// @return The name of the unresolved type.
    llvm::StringRef getName() const
    {
        return getTrailingObjects<llvm::StringRef>()[_numComponents];
    }

    ast::NamespaceIdentifier getIdentifiers() const
    {
        return ast::NamespaceIdentifier {
            llvm::ArrayRef<llvm::StringRef>(
                getTrailingObjects<llvm::StringRef>(), _numComponents
            ),
            getTrailingObjects<llvm::StringRef>()[_numComponents]
        };
    }

    llvm::ArrayRef<glu::types::TypeBase *> getTemplateArgs() const
    {
        return llvm::ArrayRef<glu::types::TypeBase *>(
            getTrailingObjects<glu::types::TypeBase *>(), _numTemplateArgs
        );
    }

    /// @brief Getter for the source location of the unresolved type.
    /// @return The source location of the unresolved type.
    SourceLocation const &getLocation() const { return _location; }

    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::UnresolvedNameTyKind;
    }
};

}

#endif // GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP
