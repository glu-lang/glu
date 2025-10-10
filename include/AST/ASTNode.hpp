#ifndef GLU_AST_ASTNODE_HPP
#define GLU_AST_ASTNODE_HPP

#include "Basic/SourceLocation.hpp"
#include "Types/TypeBase.hpp"
#include "Visibility.hpp"

#include <cassert>
#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

class ModuleDecl;

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

    /// @brief Get the location of the current node.
    /// @return The location of the current node.
    SourceLocation getLocation() const { return _nodeLocation; }

    /// @brief Get the module in which the current node is declared.
    /// @return The module in which the current node is declared.
    ModuleDecl *getModule();

    /// @brief Print a human-readable representation of this node to
    /// an output stream, when --print-ast is enabled.
    /// @param out The output stream to print to.
    void print(llvm::raw_ostream &out);

    /// @brief Print a human-readable representation of this node to
    /// standard output, for debugging purposes.
    void print();
};

/// @brief Replace a child node in its parent node
/// @param oldExpr The expression to replace
/// @param newExpr The new expression to replace it with
void replaceChild(
    ast::ASTNode *parent, ast::ASTNode *oldNode, ast::ASTNode *newNode
);

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
    types::TypeBase *_type;

protected:
    ExprBase(NodeKind kind, SourceLocation nodeLocation)
        : ASTNode(kind, nodeLocation, nullptr), _type(nullptr)
    {
        assert(
            kind > NodeKind::ExprBaseFirstKind
            && kind < NodeKind::ExprBaseLastKind
        );
    }

public:
    /// @brief Get the type of the expression.
    /// @return The type of the expression.
    types::TypeBase *getType() const { return _type; }

    /// @brief Set the type of the expression.
    /// @param type The type of the expression.
    void setType(types::TypeBase *type) { _type = type; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() >= NodeKind::ExprBaseFirstKind
            && node->getKind() <= NodeKind::ExprBaseLastKind;
    }
};

class MetadataBase : public ASTNode {
protected:
    MetadataBase(NodeKind kind, SourceLocation nodeLocation)
        : ASTNode(kind, nodeLocation, nullptr)
    {
        assert(
            kind > NodeKind::MetadataBaseFirstKind
            && kind < NodeKind::MetadataBaseLastKind
        );
    }

public:
    static bool classof(ASTNode const *node)
    {
        return node->getKind() >= NodeKind::MetadataBaseFirstKind
            && node->getKind() <= NodeKind::MetadataBaseLastKind;
    }
};

} // end namespace glu::ast

#endif // GLU_AST_ASTNODE_H
