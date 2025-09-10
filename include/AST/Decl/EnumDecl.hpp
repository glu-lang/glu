#ifndef GLU_AST_DECL_ENUMDECL_HPP
#define GLU_AST_DECL_ENUMDECL_HPP

#include "ASTContext.hpp"
#include "ASTNodeMacros.hpp"
#include "Decl/FieldDecl.hpp"
#include "TypeDecl.hpp"
#include "Types.hpp"

#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @class EnumDecl
/// @brief Represents a enum declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a enum
/// declaration.
class EnumDecl final : public TypeDecl,
                       private llvm::TrailingObjects<EnumDecl, FieldDecl *> {

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        EnumDecl, _numFields, FieldDecl *, Fields
    )

private:
    llvm::StringRef _name;
    glu::types::EnumTy *_self;

public:
    /// @brief Constructor for the EnumDecl class.
    /// @param context The AST context.
    /// @param location The source location of the enum declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the enum.
    /// @param fields A vector of FieldDecl objects representing the fields of
    /// the enum.
    /// @param visibility The visibility of the enum.
    EnumDecl(
        ASTContext &context, SourceLocation location, ASTNode *parent,
        llvm::StringRef name, llvm::ArrayRef<FieldDecl *> fields,
        Visibility visibility = Visibility::Private
    )
        : TypeDecl(NodeKind::EnumDeclKind, location, parent, visibility)
        , _name(name)
        , _self(context.getTypesMemoryArena().create<glu::types::EnumTy>(this))
    {
        initFields(fields);
    }

    static EnumDecl *create(
        llvm::BumpPtrAllocator &allocator, ASTContext &context,
        SourceLocation location, ASTNode *parent, llvm::StringRef const &name,
        llvm::ArrayRef<FieldDecl *> fields,
        Visibility visibility = Visibility::Private
    )
    {
        auto totalSize = totalSizeToAlloc<FieldDecl *>(fields.size());
        void *mem = allocator.Allocate(totalSize, alignof(EnumDecl));
        return new (mem) EnumDecl(context, location, parent, name, fields, visibility);
    }

    /// @brief Getter for the name of the enum.
    /// @return Returns the name of the enum.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the type of this enum.
    /// @return Returns the type of this enum.
    glu::types::EnumTy *getType() const { return _self; }

    /// @brief Getter for the field count of this enum.
    /// @return Returns the field count of this enum.
    size_t getFieldCount() const { return _numFields; }

    /// @brief Getter for a specific field of this enum.
    /// @param index The index of the field.
    /// @return Returns a const reference to the field at the specified index.
    FieldDecl *getField(size_t index) const
    {
        assert(index < _numFields && "Index out of bounds");
        return getFields()[index];
    }

    llvm::MutableArrayRef<FieldDecl *> getMutableFields()
    {
        return { getTrailingObjects<FieldDecl *>(), _numFields };
    }

    /// @brief Getter for the index of a specific field by name.
    /// @param name The name of the field.
    /// @return Returns the index of the field if found, or std::nullopt if not.
    std::optional<size_t> getFieldIndex(llvm::StringRef name) const
    {
        for (size_t i = 0; i < _numFields; ++i) {
            if (getField(i)->getName() == name)
                return i;
        }
        return std::nullopt;
    }

    /// @brief Static method to check if a node is a EnumDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::EnumDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_ENUMDECL_HPP
