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
        FunctionAttachment = 1 << 1,
        StructAttachment = 1 << 2,
        EnumAttachment = 1 << 3,
        TypeAliasAttachment = 1 << 4,
        TypeAttachment
        = StructAttachment | EnumAttachment | TypeAliasAttachment,
        GlobalVarAttachment = 1 << 5,
        GlobalLetAttachment = 1 << 6,
        GlobalAttachment = GlobalVarAttachment | GlobalLetAttachment,
        LocalVarAttachment = 1 << 7,
        LocalLetAttachment = 1 << 8,
        ParamAttachment = 1 << 9,
        LocalAttachment
        = LocalVarAttachment | LocalLetAttachment | ParamAttachment,
        FieldAttachment = 1 << 10,
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
#define ATTRIBUTE_KIND(Name, Lexeme, Attachment) Name##Kind,
#include "Attributes.def"
    InvalidKind
};

class Attribute : public MetadataBase {
    AttributeKind _kind;

public:
    Attribute(AttributeKind kind, SourceLocation location)
        : MetadataBase(NodeKind::AttributeKind, location), _kind(kind)
    {
    }

    AttributeKind getAttributeKind() const { return _kind; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::AttributeKind;
    }

    static AttributeKind getAttributeKindFromLexeme(llvm::StringRef lexeme)
    {
#define ATTRIBUTE_KIND(Name, Lexeme, Attachment) \
    if (lexeme == Lexeme)                        \
        return AttributeKind::Name##Kind;
#include "Attributes.def"
        return AttributeKind::InvalidKind;
    }

    llvm::StringRef getAttributeKindName() const
    {
        switch (_kind) {
#define ATTRIBUTE_KIND(Name, Lexeme, Attachment) \
case AttributeKind::Name##Kind: return #Name;
#include "Attributes.def"
        default: return "invalid";
        }
    }

    bool isValid(AttributeAttachment attachment) const
    {
        switch (_kind) {
#define ATTRIBUTE_KIND(Name, Lexeme, Attachment)                          \
case AttributeKind::Name##Kind:                                           \
    return (attachment._rawValue & AttributeAttachment::Attachment) != 0;
#include "Attributes.def"
        default: return false;
        }
    }
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
        for (auto *attr : getAttributes()) {
            if (attr->getAttributeKind() == kind)
                return attr;
        }
        return nullptr;
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::AttributeListKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_ATTRIBUTES_HPP
