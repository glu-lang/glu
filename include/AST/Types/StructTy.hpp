#ifndef GLU_AST_TYPES_STRUCTTY_HPP
#define GLU_AST_TYPES_STRUCTTY_HPP

#include "Basic/SourceLocation.hpp"
#include "TypeBase.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/AST/STLExtras.h>
#include <string>

namespace glu::types {

/// @brief StructTy is a class that represents structures declared in code.
class StructTy : public TypeBase {

    struct Field {
        std::string name;
        TypeBase *type;
    };

    std::string _name;
    llvm::SmallVector<Field> _fields;
    SourceLocation _definitionLocation;

    // TODO: Add attributes (e.g. packed, alignment)

public:
    /// @brief Constructor for the StructTy class.
    StructTy(
        std::string name, llvm::SmallVector<Field> fields,
        SourceLocation definitionLocation
    )
        : TypeBase(TypeKind::StructTyKind)
        , _name(name)
        , _fields(fields)
        , _definitionLocation(definitionLocation)
    {
    }

    std::string getName() const { return _name; }

    size_t getFieldCount() const { return _fields.size(); }
    Field const &getField(size_t index) const { return _fields[index]; }

    SourceLocation getDefinitionLocation() const { return _definitionLocation; }

    std::optional<size_t> getFieldIndex(llvm::StringRef name) const
    {
        auto it = llvm::find_if(_fields, [&](Field const &field) {
            return field.name == name;
        });
        if (it == _fields.end())
            return std::nullopt;
        return std::distance(_fields.begin(), it);
    }

    /// @brief Static method to check if a type is a StructTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::StructTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_STRUCTTY_HPP
