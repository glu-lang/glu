#include "Decls.hpp"
#include "Types.hpp"

namespace glu::types {

llvm::StringRef StructTy::getName() const
{
    return _decl->getName();
}

size_t StructTy::getFieldCount() const
{
    return _decl->getFieldCount();
}

size_t StructTy::getRequiredFieldCount()
{
    return _decl->getRequiredFieldCount();
}

SourceLocation StructTy::getLocation() const
{
    return _decl->getLocation();
}

glu::ast::FieldDecl *StructTy::getField(size_t index)
{
    return _decl->getField(index);
}

std::optional<size_t> StructTy::getFieldIndex(llvm::StringRef name) const
{
    return _decl->getFieldIndex(name);
}

llvm::ArrayRef<glu::ast::FieldDecl *> StructTy::getFields() const
{
    return _decl->getFields();
}

} // end namespace glu::types
