#include "ASTWalker.hpp"
#include "Types.hpp"

namespace glu::sema {

template <typename Impl>
class TypeMapper : public glu::ast::ASTWalker<Impl, void> {
public:
#define NODE_KIND_(NodeName, Parent, ...)                                      \
    void postVisit##NodeName([[maybe_unused]] ast::NodeName *node) { __VA_ARGS__; }
#define NODE_KIND(Name, Parent)

#define NODE_TYPEREF(Type, Name)                                            \
    node->set##Name(                                                        \
        llvm::cast<glu::types::Type>(this->asImpl()->mapType(node->get##Name())) \
    )

#define NODE_CHILD(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name) (void) 0
#include "NodeKind.def"

    // Default implementation of mapType. This method serves as a fallback for
    // derived classes and simply returns the input type unchanged. Derived
    // classes can override this method to provide custom type mapping logic.
    glu::types::TypeBase *mapType(glu::types::TypeBase *type) { return type; }
};
} // namespace glu::sema
