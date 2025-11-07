#ifndef GLU_AST_EXPR_CALLTEMPLATEARGUMENT_HPP
#define GLU_AST_EXPR_CALLTEMPLATEARGUMENT_HPP

#include "ASTNode.hpp"
#include "Types/TypeBase.hpp"

namespace glu::ast {

/// Represents a single explicit template argument attached to a call
/// expression. It currently stores the resolved type and carries a source
/// location so we can extend it later if needed.
class CallTemplateArgument final : public MetadataBase {
    glu::types::TypeBase *_type;

public:
    CallTemplateArgument(SourceLocation location, glu::types::TypeBase *type)
        : MetadataBase(NodeKind::CallTemplateArgumentKind, location)
        , _type(type)
    {
    }

    glu::types::TypeBase *getType() const { return _type; }

    void setType(glu::types::TypeBase *type) { _type = type; }
};

} // namespace glu::ast

#endif // GLU_AST_EXPR_CALLTEMPLATEARGUMENT_HPP
