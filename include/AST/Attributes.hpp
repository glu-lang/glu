#ifndef GLU_AST_ATTRIBUTES_HPP
#define GLU_AST_ATTRIBUTES_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

struct AttributeAttachment {
    enum AttributeAttachmentKind : uint64_t {
        ImportAttachment = 1 << 0,
        FunctionPrototypeAttachment = 1 << 1,
        FunctionDefinitionAttachment = 1 << 2,
        FunctionAttachment
        = FunctionPrototypeAttachment | FunctionDefinitionAttachment,
        StructAttachment = 1 << 3,
        EnumAttachment = 1 << 4,
        TypeAliasAttachment = 1 << 5,
        TypeAttachment
        = StructAttachment | EnumAttachment | TypeAliasAttachment,
        GlobalVarAttachment = 1 << 6,
        GlobalLetAttachment = 1 << 7,
        GlobalAttachment = GlobalVarAttachment | GlobalLetAttachment,
        LocalVarAttachment = 1 << 8,
        LocalLetAttachment = 1 << 9,
        ParamAttachment = 1 << 10,
        LocalAttachment
        = LocalVarAttachment | LocalLetAttachment | ParamAttachment,
        FieldAttachment = 1 << 11,
        DeclAttachment = ImportAttachment | FunctionAttachment | TypeAttachment
            | GlobalAttachment | LocalAttachment | FieldAttachment
    };
    AttributeAttachmentKind _rawValue;

public:
    AttributeAttachment(AttributeAttachmentKind rawValue) : _rawValue(rawValue)
    {
    }
};

enum class AttributeKind {
#define ATTRIBUTE_KIND(Name, ...) Name##Kind,
#include "Attributes.def"
    InvalidKind
};

class Attribute : public MetadataBase {
    AttributeKind _kind;

    GLU_AST_GEN_CHILD(Attribute, ExprBase *, _parameter, Parameter)

public:
    Attribute(
        AttributeKind kind, SourceLocation location,
        ExprBase *parameter = nullptr
    )
        : MetadataBase(NodeKind::AttributeKind, location), _kind(kind)
    {
        initParameter(parameter, /*nullable = */ true);
    }

    AttributeKind getAttributeKind() const { return _kind; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::AttributeKind;
    }

    static AttributeKind getAttributeKindFromLexeme(llvm::StringRef lexeme)
    {
#define ATTRIBUTE_KIND(Name, Lexeme, ...) \
    if (lexeme == Lexeme)                 \
        return AttributeKind::Name##Kind;
#include "Attributes.def"
        return AttributeKind::InvalidKind;
    }

    llvm::StringRef getAttributeKindName() const
    {
        switch (_kind) {
#define ATTRIBUTE_KIND(Name, ...)             \
case AttributeKind::Name##Kind: return #Name;
#include "Attributes.def"
        default: return "invalid";
        }
    }

    llvm::StringRef getAttributeKindSpelling() const
    {
        switch (_kind) {
#define ATTRIBUTE_KIND(Name, Lexeme, ...)      \
case AttributeKind::Name##Kind: return Lexeme;
#include "Attributes.def"
        default: return "invalid";
        }
    }

    bool isValidOn(AttributeAttachment attachment) const
    {
        switch (_kind) {
#define ATTRIBUTE_KIND(Name, Lexeme, Attachment, ...)                 \
case AttributeKind::Name##Kind:                                       \
    return (attachment._rawValue & (AttributeAttachment::Attachment)) \
        == attachment._rawValue;
#include "Attributes.def"
        default: return false;
        }
    }

    bool isValidOnOneOf(AttributeAttachment attachment) const
    {
        switch (_kind) {
#define ATTRIBUTE_KIND(Name, Lexeme, Attachment, ...)                       \
case AttributeKind::Name##Kind:                                             \
    return (attachment._rawValue & (AttributeAttachment::Attachment)) != 0;
#include "Attributes.def"
        default: return false;
        }
    }

    /// @brief Check if this attribute expects a parameter.
    bool expectsParameter() const;

    /// @brief Check if the given expression is a valid parameter type for this
    /// attribute.
    bool isValidParameterType(ExprBase *expr) const;

    /// @brief Get the expected parameter type name for this attribute (for
    /// diagnostics).
    llvm::StringRef getExpectedParameterTypeName() const;
};

class AttributeList final
    : public MetadataBase,
      public llvm::TrailingObjects<AttributeList, Attribute *> {

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        AttributeList, _numAttributes, Attribute *, Attributes
    )

public:
    AttributeList(
        llvm::ArrayRef<Attribute *> attributes, SourceLocation location
    )
        : MetadataBase(NodeKind::AttributeListKind, location)
    {
        initAttributes(attributes);
    }

    static AttributeList *create(
        llvm::BumpPtrAllocator &alloc, llvm::ArrayRef<Attribute *> attributes,
        SourceLocation location
    )
    {
        void *mem = alloc.Allocate(
            totalSizeToAlloc<Attribute *>(attributes.size()),
            alignof(AttributeList)
        );
        return new (mem) AttributeList(attributes, location);
    }

    /// @brief Get an attribute by its kind.
    /// @param kind The kind of the attribute to retrieve.
    /// @return The attribute if found, or nullptr.
    Attribute *getAttribute(AttributeKind kind) const
    {
        auto it = llvm::find_if(getAttributes(), [kind](auto const *attr) {
            return attr->getAttributeKind() == kind;
        });
        return it != getAttributes().end() ? *it : nullptr;
    }

    bool hasAttribute(AttributeKind kind) const
    {
        return getAttribute(kind) != nullptr;
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::AttributeListKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_ATTRIBUTES_HPP
