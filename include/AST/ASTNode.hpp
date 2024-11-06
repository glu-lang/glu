#ifndef GLU_AST_ASTNODE_HPP
#define GLU_AST_ASTNODE_HPP

#include "SourceLocation.hpp"

namespace glu::ast {

/**
 * @brief The kind of a node in the AST.
 **/
enum class NodeKind {
#define NODE_KIND(Name, Parent) Name##Kind,
#define NODE_KIND_SUPER(Name, Parent) Name##FirstKind,
#define NODE_KIND_SUPER_END(Name) Name##LastKind,
#include "NodeKind.def"
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
    ASTNode(
        NodeKind kind, SourceLocation nodeLocation, ASTNode *parent = nullptr
    )
        : _parent(parent), _nodeKind(kind), _nodeLocation(nodeLocation)
    {
    }
    ~ASTNode() = default;

    /**
     * @brief Get the kind of the current node.
     * @return The kind of the current node.
     *
     **/
    NodeKind getKind() const { return _nodeKind; }
};

}

#endif // GLU_AST_ASTNODE_H
