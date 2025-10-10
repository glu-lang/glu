#include "AST/Attributes.hpp"
#include "Exprs.hpp"

namespace glu::ast {

bool Attribute::expectsParameter() const
{
    switch (_kind) {
#define ATTRIBUTE_KIND(Name, ...)                 \
    case AttributeKind::Name##Kind: return false;
#define ATTRIBUTE_WITH_PARAM(Name, ...)          \
    case AttributeKind::Name##Kind: return true;
#include "AST/Attributes.def"
    default: return false;
    }
}

bool Attribute::isValidParameterType(ExprBase *expr) const
{
    if (!expr)
        return !expectsParameter();

    switch (_kind) {
#define ATTRIBUTE_KIND(Name, ...)                 \
    case AttributeKind::Name##Kind: return false;
#define ATTRIBUTE_WITH_PARAM(Name, Lexeme, Attachment, ParamType)      \
    case AttributeKind::Name##Kind: return llvm::isa<ParamType>(expr);
#include "AST/Attributes.def"
    default: return false;
    }
}

llvm::StringRef Attribute::getExpectedParameterTypeName() const
{
    switch (_kind) {
#define ATTRIBUTE_KIND(Name, ...)                  \
    case AttributeKind::Name##Kind: return "none";
#define ATTRIBUTE_WITH_PARAM(Name, Lexeme, Attachment, ParamType) \
    case AttributeKind::Name##Kind: return #ParamType;
#include "AST/Attributes.def"
    default: return "unknown";
    }
}

} // namespace glu::ast
