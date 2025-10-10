#include "Decls.hpp"
#include "Exprs.hpp"
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

bool StructTy::isPacked() const
{
    return _decl->hasAttribute(ast::AttributeKind::PackedKind);
}

uint64_t StructTy::getAlignment() const
{
    auto *attribute = _decl->getAttribute(ast::AttributeKind::AlignmentKind);
    if (!attribute)
        return 0;

    if (auto *literal
        = llvm::dyn_cast_if_present<ast::LiteralExpr>(attribute->getParameter()
        )) {
        if (std::holds_alternative<llvm::APInt>(literal->getValue())) {
            return std::get<llvm::APInt>(literal->getValue()).getZExtValue();
        }
    }
    return 0;
}

} // end namespace glu::types
