#include "ASTWalker.hpp"
#include "Types.hpp"

namespace glu::sema {

template <typename Impl>
class TypeMapper : public glu::ast::ASTWalker<Impl, void> {

#define NODE_KIND_(NodeName, Parent, ...)                     \
    void postVisit##NodeName(NodeName *node) { __VA_ARGS__; }
#define NODE_KIND(Name, Parent)

#define NODE_TYPEREF(Type, Name)                                \
    node->set##Name(this->asImpl()._mapType(node->get##Name()))

#define NODE_CHILD(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name) (void) 0

    glu::types::TypeBase *_mapType(glu::types::TypeBase *type)
    {
        return this->asImpl()->mapType(type);
    }

    glu::types::FunctionTy *_mapType(glu::types::FunctionTy *type)
    {
        return llvm::cast<glu::types::FunctionTy>(this->asImpl()->mapType(type));
    }

    glu::types::TypeBase *mapType(glu::types::TypeBase *type) { return type; }
};

} // namespace glu::sema
