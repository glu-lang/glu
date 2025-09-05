#ifndef GLU_AST_TYPES_ENUMTY_HPP
#define GLU_AST_TYPES_ENUMTY_HPP

#include "Basic/SourceLocation.hpp"
#include "TypeBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

namespace glu::ast {
class EnumDecl;
class FieldDecl;
} // end namespace glu::ast

namespace glu::types {

class EnumTy final : public TypeBase {
private:
    glu::ast::EnumDecl *_decl;

public:
    EnumTy(glu::ast::EnumDecl *decl)
        : TypeBase(TypeKind::EnumTyKind), _decl(decl)
    {
    }

    glu::ast::EnumDecl *getDecl() const;

    llvm::StringRef getName() const;

    size_t getFieldCount() const;

    SourceLocation getLocation() const;

    glu::ast::FieldDecl *getField(size_t index) const;

    std::optional<size_t> getFieldIndex(llvm::StringRef name) const;

    /// @brief Returns the array of fields.
    llvm::ArrayRef<glu::ast::FieldDecl *> getFields() const;

    /// @brief Static method to check if a type is an EnumTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::EnumTyKind;
    }
};

} // namespace glu::types

#endif // GLU_AST_TYPES_ENUMTY_HPP
