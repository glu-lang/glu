#ifndef GLU_AST_DECL_STRUCTDECL_HPP
#define GLU_AST_DECL_STRUCTDECL_HPP

#include "ASTContext.hpp"
#include "TypeDecl.hpp"
#include "Types.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class StructDecl
/// @brief Represents a struct declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a struct
/// declaration.
using Field = glu::types::Field;
using StructTy = glu::types::StructTy;

// TODO: Add attributes (e.g. packed, alignment)
class StructDecl final : public TypeDecl,
                         private llvm::TrailingObjects<StructDecl, Field> {
public:
    friend llvm::TrailingObjects<StructDecl, Field>;

private:
    llvm::StringRef _name;
    unsigned _numFields;
    StructTy *_self;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(
        typename llvm::TrailingObjects<StructDecl, Field>::OverloadToken<Field>
    ) const
    {
        return _numFields;
    }

public:
    /// @brief Constructor for the StructDecl class.
    /// @param context The AST context.
    /// @param location The source location of the struct declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the struct.
    /// @param params A vector of Field objects representing the fields of
    /// the struct.
    StructDecl(
        ASTContext &context, SourceLocation location, ASTNode *parent,
        llvm::StringRef name, llvm::ArrayRef<Field> fields
    )
        : TypeDecl(NodeKind::StructDeclKind, location, parent)
        , _name(name)
        , _numFields(fields.size())
        , _self(context.getTypesMemoryArena().create<StructTy>(this))
    {
        std::uninitialized_copy(
            fields.begin(), fields.end(),
            this->template getTrailingObjects<Field>()
        );
    }

    static StructDecl *create(
        llvm::BumpPtrAllocator &allocator, ASTContext &context,
        SourceLocation location, ASTNode *parent, llvm::StringRef const &name,
        llvm::ArrayRef<Field> fields
    )
    {
        auto totalSize = totalSizeToAlloc<Field>(fields.size());
        void *mem = allocator.Allocate(totalSize, alignof(StructDecl));
        return new (mem) StructDecl(context, location, parent, name, fields);
    }

    /// @brief Getter for the name of the struct.
    /// @return Returns the name of the struct.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the type of this struct.
    /// @return Returns the type of this struct.
    StructTy *getType() const { return _self; }

    /// @brief Getter for the field count of this struct.
    /// @return Returns the field count of this struct.
    size_t getFieldCount() const { return _numFields; }

    /// @brief Getter for a specific field of this struct.
    /// @param index The index of the field.
    /// @return Returns a const reference to the field at the specified index.
    Field const &getField(size_t index) const
    {
        assert(index < _numFields && "Index out of bounds");
        return getTrailingObjects<Field>()[index];
    }

    /// @brief Getter for the index of a specific field by name.
    /// @param name The name of the field.
    /// @return Returns the index of the field if found, or std::nullopt if not.
    std::optional<size_t> getFieldIndex(llvm::StringRef name) const
    {
        for (size_t i = 0; i < _numFields; ++i) {
            if (getField(i).name == name)
                return i;
        }
        return std::nullopt;
    }

    /// @brief Getter for the array of fields.
    /// @return Returns an ArrayRef to the fields of this struct.
    llvm::MutableArrayRef<Field> getFields()
    {
        return { getTrailingObjects<Field>(), _numFields };
    }

    /// @brief Static method to check if a node is a StructDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::StructDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_STRUCTDECL_HPP
