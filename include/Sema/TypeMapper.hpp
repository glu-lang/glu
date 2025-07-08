#include "ASTWalker.hpp"

namespace glu::sema {

template <typename Impl>
class TypeMapper : public glu::ast::ASTWalker<TypeMapper<Impl>, void> {
    Impl asImpl() { return static_cast<Impl &>(*this); }

#define NODE_KIND_(NodeName, Parent, ...)                     \
    void postVisit##NodeName(NodeName *node) { __VA_ARGS__; }
#define NODE_KIND(Name, Parent)

#define NODE_TYPEREF(Type, Name)                                \
    node->set##Name(this->asImpl()._mapType(node->get##Name()))

#define NODE_CHILD(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name) (void) 0

    TypeBase *_mapType(TypeBase *type) { return mapType(type); }

    FunctionTy *_mapType(FunctionTy *type)
    {
        return llvm::cast<FunctionTy>(mapType(type));
    }

    TypeBase *mapType(TypeBase *type) { return type; }
};
}
