#include "AST/TypeTransformer.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Types.hpp"

namespace glu::types {

namespace {

/// @brief A type transformer that substitutes template parameters with their
/// concrete types from a parameterized struct type.
class TemplateParamSubstituter
    : public TypeTransformerBase<TemplateParamSubstituter> {
    StructTy *_structType;

public:
    TemplateParamSubstituter(ast::ASTContext *context, StructTy *structType)
        : TypeTransformerBase(context), _structType(structType)
    {
    }

    TypeBase *visitTemplateParamTy(TemplateParamTy *type)
    {
        auto *decl = _structType->getDecl();
        auto *templateParams = decl->getTemplateParams();
        if (!templateParams)
            return type;

        auto params = templateParams->getTemplateParameters();
        auto args = _structType->getTemplateArgs();

        for (size_t i = 0; i < params.size() && i < args.size(); ++i) {
            if (params[i]->getType() == type) {
                return visit(args[i]);
            }
        }
        return type;
    }
};

} // namespace

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

    if (auto *literal = llvm::dyn_cast_if_present<ast::LiteralExpr>(
            attribute->getParameter()
        )) {
        if (std::holds_alternative<llvm::APInt>(literal->getValue())) {
            return std::get<llvm::APInt>(literal->getValue()).getZExtValue();
        }
    }
    return 0;
}

TypeBase *StructTy::getSubstitutedFieldType(size_t index)
{
    auto *field = getField(index);
    if (getTemplateArgs().empty())
        return field->getType();

    auto *ctx = _decl->getModule()->getContext();
    TemplateParamSubstituter substituter(ctx, this);
    return substituter.visit(field->getType());
}

} // end namespace glu::types
