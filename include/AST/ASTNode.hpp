#ifndef GLU_AST_ASTNODE_HPP
#define GLU_AST_ASTNODE_HPP

#include "Basic/SourceLocation.hpp"

#include <cassert>

namespace glu::ast {

///
/// @brief The kind of a node in the AST.
///
enum class NodeKind {
#define NODE_KIND(Name, Parent) Name##Kind,
#define NODE_KIND_SUPER(Name, Parent) Name##FirstKind,
#define NODE_KIND_SUPER_END(Name) Name##LastKind,
#include "NodeKind.def"
};

///
/// @brief The base class for all nodes in the AST.
///
/// This class provides the basic functionality for all nodes in the AST.
/// It provides a pointer to the parent node, the location of the node in the
/// source code, and the kind of the node.
///
class ASTNode {
    /// A pointer to the parent node of the current node.
    /// nullptr if the current node is the root of the AST.
    ASTNode *_parent;

    /// The location from which the node was parsed.
    SourceLocation _nodeLocation;

    /// The AST node kind of the current node.
    NodeKind _nodeKind;

protected:
    ASTNode(
        NodeKind kind, SourceLocation nodeLocation, ASTNode *parent = nullptr
    )
        : _parent(parent), _nodeLocation(nodeLocation), _nodeKind(kind)
    {
    }

public:
    /// @brief Get the kind of the current node.
    /// @return The kind of the current node.
    NodeKind getKind() const { return _nodeKind; }

    /// @brief Set the parent of the current node.
    /// @param parent The parent node of the current node.
    void setParent(ASTNode *parent) { _parent = parent; }

    /// @brief Get the parent of the current node.
    /// @return The parent node of the current node.
    ASTNode *getParent() const { return _parent; }
};

class DeclBase : public ASTNode {
protected:
    DeclBase(
        NodeKind kind, SourceLocation nodeLocation, ASTNode *parent = nullptr
    )
        : ASTNode(kind, nodeLocation, parent)
    {
        assert(
            kind > NodeKind::DeclBaseFirstKind
            && kind < NodeKind::DeclBaseLastKind
        );
    }

public:
    static bool classof(ASTNode const *node)
    {
        return node->getKind() >= NodeKind::DeclBaseFirstKind
            && node->getKind() <= NodeKind::DeclBaseLastKind;
    }
};

class StmtBase : public ASTNode {
protected:
    StmtBase(NodeKind kind, SourceLocation nodeLocation)
        : ASTNode(kind, nodeLocation, nullptr)
    {
        assert(
            kind > NodeKind::StmtBaseFirstKind
            && kind < NodeKind::StmtBaseLastKind
        );
    }

public:
    static bool classof(ASTNode const *node)
    {
        return node->getKind() >= NodeKind::StmtBaseFirstKind
            && node->getKind() <= NodeKind::StmtBaseLastKind;
    }
};

class ExprBase : public ASTNode {
protected:
    ExprBase(NodeKind kind, SourceLocation nodeLocation)
        : ASTNode(kind, nodeLocation, nullptr)
    {
        assert(
            kind > NodeKind::ExprBaseFirstKind
            && kind < NodeKind::ExprBaseLastKind
        );
    }

public:
    static bool classof(ASTNode const *node)
    {
        return node->getKind() >= NodeKind::ExprBaseFirstKind
            && node->getKind() <= NodeKind::ExprBaseLastKind;
    }
};

} // end namespace glu::ast

#endif // GLU_AST_ASTNODE_H
