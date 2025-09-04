#ifndef GLU_AST_TYPES_STRUCTTY_HPP
#define GLU_AST_TYPES_STRUCTTY_HPP

#include "Basic/SourceLocation.hpp"
#include "TypeBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

namespace glu::ast {
class StructDecl;
class FieldDecl;
} // end namespace glu::ast

namespace glu::types {

/// @brief StructTy is a class that represents structures declared in code.
class StructTy final : public TypeBase {
private:
    glu::ast::StructDecl *_decl;

public:
    StructTy(glu::ast::StructDecl *decl)
        : TypeBase(TypeKind::StructTyKind), _decl(decl)
    {
    }

    glu::ast::StructDecl *getDecl() const { return _decl; }

    llvm::StringRef getName() const;

    size_t getFieldCount() const;

    SourceLocation getLocation() const;

    glu::ast::FieldDecl *getField(size_t index);

    std::optional<size_t> getFieldIndex(llvm::StringRef name) const;

    /// @brief Returns the array of fields.
    llvm::ArrayRef<glu::ast::FieldDecl *> getFields() const;

    /// @brief Static method to check if a type is a StructTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::StructTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_STRUCTTY_HPP
