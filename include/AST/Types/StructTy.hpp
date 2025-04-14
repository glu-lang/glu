#ifndef GLU_AST_TYPES_STRUCTTY_HPP
#define GLU_AST_TYPES_STRUCTTY_HPP

#include "Basic/SourceLocation.hpp"
#include "TypeBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace glu::types {

struct Field {
    llvm::StringRef name;
    TypeBase *type;
};

/// @brief StructTy is a class that represents structures declared in code.
class StructTy final : public TypeBase,
                       private llvm::TrailingObjects<StructTy, Field> {
public:
    friend llvm::TrailingObjects<StructTy, Field>;

    // TODO: Add attributes (e.g. packed, alignment)
private:
    llvm::StringRef _name;
    unsigned _numFields;
    SourceLocation _definitionLocation;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename llvm::TrailingObjects<StructTy, Field>::OverloadToken<Field>) const
    {
        return _numFields;
    }

    StructTy(llvm::StringRef name, unsigned numFields, SourceLocation loc)
        : TypeBase(TypeKind::StructTyKind)
        , _name(name)
        , _numFields(numFields)
        , _definitionLocation(loc)
    {
    }

public:
    /// @brief Constructor for the StructTy class.
    StructTy(
        llvm::StringRef const &name, llvm::ArrayRef<Field> const &fields,
        SourceLocation definitionLocation
    )
        : StructTy(name, fields.size(), definitionLocation)
    {
    }

    static StructTy *create(
        llvm::BumpPtrAllocator &allocator, llvm::StringRef const &name,
        llvm::ArrayRef<Field> fields, SourceLocation definitionLocation
    )
    {
        auto totalSize = totalSizeToAlloc<Field>(fields.size());
        void *mem = allocator.Allocate(totalSize, alignof(StructTy));
        StructTy *s = new (mem) StructTy(name, fields, definitionLocation);
        std::uninitialized_copy(
            fields.begin(), fields.end(),
            s->template getTrailingObjects<Field>()
        );
        return s;
    }

    llvm::StringRef const &getName() const { return _name; }

    size_t getFieldCount() const { return _numFields; }

    SourceLocation getDefinitionLocation() const { return _definitionLocation; }

    Field const &getField(size_t index) const
    {
        assert(index < _numFields && "Index out of bounds");
        return this->template getTrailingObjects<Field>()[index];
    }

    std::optional<size_t> getFieldIndex(llvm::StringRef name) const
    {
        for (size_t i = 0; i < _numFields; ++i) {
            if (getField(i).name == name)
                return i;
        }
        return std::nullopt;
    }

    /// @brief Returns the array of fields.
    llvm::ArrayRef<Field> getFields() const
    {
        return llvm::ArrayRef(
            this->template getTrailingObjects<Field>(), _numFields
        );
    }

    /// @brief Static method to check if a type is a StructTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::StructTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_STRUCTTY_HPP
