#ifndef GLU_AST_DECL_STRUCTDECL_HPP
#define GLU_AST_DECL_STRUCTDECL_HPP

#include "ASTContext.hpp"
#include "ASTNodeMacros.hpp"
#include "Decl/FieldDecl.hpp"
#include "Decl/FunctionDecl.hpp"
#include "Decl/TemplateParameterDecl.hpp"
#include "TypeDecl.hpp"
#include "Types.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class StructDecl
/// @brief Represents a struct declaration in the AST.
///
/// This class inherits from DeclBase and encapsulates the details of a struct
/// declaration.

class StructDecl final
    : public TypeDecl,
      private llvm::TrailingObjects<StructDecl, FieldDecl *> {
    GLU_AST_GEN_CHILD(
        StructDecl, TemplateParameterList *, _templateParams, TemplateParams
    )
    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        StructDecl, _numFields, FieldDecl *, Fields
    )
private:
    llvm::StringRef _name;
    glu::types::StructTy *_self;

    // Ownership

    /// The `drop` function for this struct, if an overload exists, nullptr
    /// otherwise.
    FunctionDecl *_dropFn = nullptr;

    /// The `copy` function for this struct, if an overload exists, nullptr
    /// otherwise.
    FunctionDecl *_copyFn = nullptr;

public:
    /// @brief Constructor for the StructDecl class.
    /// @param context The AST context.
    /// @param location The source location of the struct declaration.
    /// @param parent The parent AST node.
    /// @param name The name of the struct.
    /// @param fields A vector of FieldDecl objects representing the fields of
    /// the struct.
    /// @param visibility The visibility of the struct.
    StructDecl(
        ASTContext &context, SourceLocation location, ASTNode *parent,
        llvm::StringRef name, llvm::ArrayRef<FieldDecl *> fields,
        TemplateParameterList *templateParams, Visibility visibility,
        AttributeList *attributes
    )
        : TypeDecl(
              NodeKind::StructDeclKind, location, parent, visibility, attributes
          )
        , _name(name)
        , _self(context.getTypesMemoryArena().create<types::StructTy>(this))
    {
        initTemplateParams(templateParams, /* nullable = */ true);
        initFields(fields);
    }

    static StructDecl *create(
        llvm::BumpPtrAllocator &allocator, ASTContext &context,
        SourceLocation location, ASTNode *parent, llvm::StringRef const &name,
        llvm::ArrayRef<FieldDecl *> fields,
        TemplateParameterList *templateParams = nullptr,
        Visibility visibility = Visibility::Private,
        AttributeList *attributes = nullptr
    )
    {
        auto totalSize = totalSizeToAlloc<FieldDecl *>(fields.size());
        void *mem = allocator.Allocate(totalSize, alignof(StructDecl));
        return new (mem) StructDecl(
            context, location, parent, name, fields, templateParams, visibility,
            attributes
        );
    }

    /// @brief Getter for the name of the struct.
    /// @return Returns the name of the struct.
    llvm::StringRef getName() const { return _name; }

    /// @brief Getter for the type of this struct.
    /// @return Returns the type of this struct.
    glu::types::StructTy *getType() const { return _self; }

    /// @brief Getter for the field count of this struct.
    /// @return Returns the field count of this struct.
    size_t getFieldCount() const { return _numFields; }

    size_t getRequiredFieldCount()
    {
        size_t requiredCount = 0;
        while (requiredCount < _numFields
               && getField(requiredCount)->getValue() == nullptr) {
            ++requiredCount;
        }
        return requiredCount;
    }

    /// @brief Getter for a specific field of this struct.
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

    /// @brief Get the drop function for this struct, if it exists.
    /// @return The drop function, or nullptr if none exists.
    FunctionDecl *getDropFunction() const { return _dropFn; }

    /// @brief Set the drop function for this struct.
    /// @param dropFn The drop function to set.
    void setDropFunction(FunctionDecl *dropFn) { _dropFn = dropFn; }

    /// @brief Check if this struct has an overloaded drop function.
    /// @return True if a drop function is set, false otherwise.
    bool hasOverloadedDropFunction() const { return _dropFn != nullptr; }

    /// @brief Get the copy function for this struct, if it exists.
    /// @return The copy function, or nullptr if none exists.
    FunctionDecl *getCopyFunction() const { return _copyFn; }

    /// @brief Set the copy function for this struct.
    /// @param copyFn The copy function to set.
    void setCopyFunction(FunctionDecl *copyFn) { _copyFn = copyFn; }

    /// @brief Check if this struct has an overloaded copy function.
    /// @return True if a copy function is set, false otherwise.
    bool hasOverloadedCopyFunction() const { return _copyFn != nullptr; }

    bool isTrivial() const
    {
        // A struct is trivial if it has no overloaded copy/move/drop functions
        // and all its fields are trivial.
        if (_dropFn || _copyFn)
            return false;
        for (auto *field : getFields()) {
            if (!field->getType()->isTrivial())
                return false;
        }
        return true;
    }

    /// @brief Static method to check if a node is a StructDecl.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::StructDeclKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_STRUCTDECL_HPP
