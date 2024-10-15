#include "SourceLocation.hpp"

namespace glu::ast {

/**
 * @brief The kind of a node in the AST.
 **/
enum class NodeKind {
#define NODE_KIND(Name) Name##Kind,
#define NODE_KIND_SUPER(Name, Parent) Name##FirstKind,
#define NODE_KIND_SUPER_END(Name) Name##LastKind,
#include "NodeKind.def"
#undef NODE_KIND
#undef NODE_KIND_SUPER
#undef NODE_KIND_SUPER_END
};

/**
 * @brief The base class for all nodes in the AST.
 *
 * This class provides the basic functionality for all nodes in the AST.
 * It provides a pointer to the parent node, the location of the node in the
 * source code, and the kind of the node.
 *
 **/
class ASTNode {
    /// A pointer to the parent node of the current node.
    /// nullptr if the current node is the root of the AST.
    ASTNode *_parent;

    /// The location from which the node was parsed.
    SourceLocation _nodeLocation;

    /// The AST node kind of the current node.
    NodeKind _nodeKind;

public:
    ASTNode();
    ~ASTNode() = default;
};

}
