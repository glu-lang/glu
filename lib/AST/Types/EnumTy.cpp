#include "Decls.hpp"
#include "Types.hpp"

namespace glu::types {

llvm::StringRef EnumTy::getName() const
{
    return _decl->getName();
}

size_t EnumTy::getFieldCount() const
{
    return _decl->getFieldCount();
}

SourceLocation EnumTy::getLocation() const
{
    return _decl->getLocation();
}

glu::ast::FieldDecl *EnumTy::getField(size_t index) const
{
    return _decl->getField(index);
}

std::optional<size_t> EnumTy::getFieldIndex(llvm::StringRef name) const
{
    return _decl->getFieldIndex(name);
}

llvm::ArrayRef<glu::ast::FieldDecl *> EnumTy::getFields() const
{
    return _decl->getFields();
}

glu::types::TypeBase *EnumTy::getRepresentableType() const
{
    return _decl->getRepresentableType();
}

} // end namespace glu::types
