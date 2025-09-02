#include "AST/Types/TypeBase.hpp"
#include "AST/CanonicalTypeTransformer.hpp"

namespace glu::types {

TypeBase *TypeBase::getCanonicalType(glu::ast::ASTContext &context)
{
    return glu::ast::CanonicalTypeTransformer(context).visit(this);
}

} // namespace glu::types
