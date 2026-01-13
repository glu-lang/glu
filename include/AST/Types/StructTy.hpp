#ifndef GLU_AST_TYPES_STRUCTTY_HPP
#define GLU_AST_TYPES_STRUCTTY_HPP

#include "Basic/SourceLocation.hpp"
#include "TypeBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/TrailingObjects.h>

#include <memory>

namespace glu::ast {
class StructDecl;
class FieldDecl;
} // end namespace glu::ast

namespace glu::types {

/// @brief StructTy is a class that represents structures declared in code.
class StructTy final : public TypeBase,
                       private llvm::TrailingObjects<StructTy, TypeBase *> {
    using TrailingArgs = llvm::TrailingObjects<StructTy, TypeBase *>;
    friend TrailingArgs;

private:
    glu::ast::StructDecl *_decl;
    unsigned _numTemplateArgs;

public:
    StructTy(
        glu::ast::StructDecl *decl,
        llvm::ArrayRef<glu::types::TypeBase *> templateArgs = {}
    )
        : TypeBase(TypeKind::StructTyKind)
        , _decl(decl)
        , _numTemplateArgs(templateArgs.size())
    {
        std::uninitialized_copy(
            templateArgs.begin(), templateArgs.end(),
            getTrailingObjects<TypeBase *>()
        );
    }

    static StructTy *create(
        llvm::BumpPtrAllocator &allocator, glu::ast::StructDecl *decl,
        llvm::ArrayRef<glu::types::TypeBase *> templateArgs = {}
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<TypeBase *>(templateArgs.size()), alignof(StructTy)
        );
        return new (mem) StructTy(decl, templateArgs);
    }

    glu::ast::StructDecl *getDecl() const { return _decl; }

    llvm::StringRef getName() const;

    size_t getFieldCount() const;

    size_t getRequiredFieldCount();

    SourceLocation getLocation() const;

    glu::ast::FieldDecl *getField(size_t index);

    std::optional<size_t> getFieldIndex(llvm::StringRef name) const;

    /// @brief Returns the array of fields.
    llvm::ArrayRef<glu::ast::FieldDecl *> getFields() const;

    bool isPacked() const;

    uint64_t getAlignment() const;

    /// @brief Returns the field type with template parameters substituted.
    /// @param index The index of the field.
    /// @return The field type with template parameters replaced by concrete
    /// types.
    TypeBase *getSubstitutedFieldType(size_t index);

    size_t numTrailingObjects(TrailingArgs::OverloadToken<TypeBase *>) const
    {
        return _numTemplateArgs;
    }

    llvm::ArrayRef<TypeBase *> getTemplateArgs() const
    {
        return llvm::ArrayRef<TypeBase *>(
            getTrailingObjects<TypeBase *>(), _numTemplateArgs
        );
    }

    /// @brief Returns the type of a field with template parameters substituted.
    /// @param index The index of the field.
    /// @return The field type with template parameters replaced by concrete
    /// types.
    TypeBase *getSubstitutedFieldType(size_t index);

    /// @brief Static method to check if a type is a StructTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::StructTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_STRUCTTY_HPP
