#include "ASTWalker.hpp"

namespace glu::sema {

template <typename Impl>
class TypeMapper : public glu::ast::ASTWalker<TypeMapper<Impl>, void> {
    Impl asImpl() { return static_cast<Impl &>(*this); }

#define NODE_KIND_(Name, Parent, ...)                       \
    NODE_KIND_SUPER(NodeName, Parent)                       \
    NODE_TYPEREF(Type, TypeName)                            \
    void _visit##NodeName(NodeName *node) { \
        this->asImpl()->mapType##Type(node->get##TypeName)  \
    }
#define NODE_KIND(Name, Parent)

#define NODE_TYPEREF(Type, Name)                                    \
    void mapType##Name(Name *type) { return type; }
};
}
