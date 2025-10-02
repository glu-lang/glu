#ifndef GLU_AST_DECL_DECLBASE_HPP
#define GLU_AST_DECL_DECLBASE_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"
#include "Attributes.hpp"
#include "Visibility.hpp"

#include <cassert>

namespace glu::ast {

class DeclBase : public ASTNode {
private:
    /// @brief The visibility of this declaration.
    Visibility _visibility;
    /// @brief The attributes attached to this declaration.
    GLU_AST_GEN_CHILD(DeclBase, AttributeList *, _attributes, Attributes)

protected:
    DeclBase(
        NodeKind kind, SourceLocation nodeLocation, ASTNode *parent = nullptr,
        Visibility visibility = Visibility::Private,
        AttributeList *attributes = nullptr
    )
        : ASTNode(kind, nodeLocation, parent)
        , _visibility(visibility)
        , _attributes(attributes)
    {
        assert(
            kind > NodeKind::DeclBaseFirstKind
            && kind < NodeKind::DeclBaseLastKind
        );
        initAttributes(attributes, /* nullable = */ true);
    }

public:
    /// @brief Get the visibility of this declaration.
    /// @return The visibility of this declaration.
    Visibility getVisibility() const { return _visibility; }

    /// @brief Set the visibility of this declaration.
    /// @param visibility The new visibility.
    void setVisibility(Visibility visibility) { _visibility = visibility; }

    /// @brief Check if this declaration is public.
    /// @return True if the declaration is public, false otherwise.
    bool isPublic() const { return _visibility == Visibility::Public; }

    /// @brief Check if this declaration is private.
    /// @return True if the declaration is private, false otherwise.
    bool isPrivate() const { return _visibility == Visibility::Private; }

    Attribute *getAttribute(AttributeKind kind) const
    {
        if (!_attributes)
            return nullptr;
        return _attributes->getAttribute(kind);
    }

    bool hasAttribute(AttributeKind kind) const
    {
        return _attributes && _attributes->hasAttribute(kind);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() >= NodeKind::DeclBaseFirstKind
            && node->getKind() <= NodeKind::DeclBaseLastKind;
    }
};

} // end namespace glu::ast

#endif // GLU_AST_DECL_DECLBASE_HPP
