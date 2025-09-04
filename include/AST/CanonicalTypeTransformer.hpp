#ifndef GLU_AST_CANONICAL_TYPE_TRANSFORMER_HPP
#define GLU_AST_CANONICAL_TYPE_TRANSFORMER_HPP

#include "AST/TypeTransformer.hpp"
#include "Basic/SourceLocation.hpp"

namespace glu::ast {

class CanonicalTypeTransformer
    : public glu::types::TypeTransformerBase<CanonicalTypeTransformer> {
public:
    CanonicalTypeTransformer(ASTContext &context)
        : glu::types::TypeTransformerBase<CanonicalTypeTransformer>(&context)
    {
    }

    glu::types::TypeBase *visitTypeAliasTy(glu::types::TypeAliasTy *type)
    {
        return visit(type->getWrappedType());
    }
};

} // namespace glu::ast

#endif // GLU_AST_CANONICAL_TYPE_TRANSFORMER_HPP
