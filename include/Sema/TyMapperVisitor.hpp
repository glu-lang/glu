#ifndef GLU_SEMA_TY_MAPPER_VISITOR_HPP
#define GLU_SEMA_TY_MAPPER_VISITOR_HPP

#include "AST/TypeTransformer.hpp"
#include "Basic/Diagnostic.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

/// A reusable type mapping base for substituting/rewriting types using a
/// solution.
template <typename Derived>
class TypeMappingVisitorBase : public glu::sema::TypeMapper<Derived>,
                               public glu::types::TypeTransformerBase<Derived> {
public:
    SourceLocation _location = SourceLocation::invalid;

    TypeMappingVisitorBase(glu::ast::ASTContext *context)
        : glu::types::TypeTransformerBase<Derived>(context)
    {
    }

    // Make TypeVisitor and TypeMapper functions visible to derived classes
    using glu::types::TypeVisitor<Derived, glu::types::TypeBase *>::visit;
    using glu::sema::TypeMapper<Derived>::visit;

    glu::types::TypeBase *
    mapType(glu::types::TypeBase *type, ast::ASTNode *node)
    {
        _location = node->getLocation();
        if (type == nullptr)
            return nullptr;
        return visit(type);
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_TY_MAPPER_VISITOR_HPP
