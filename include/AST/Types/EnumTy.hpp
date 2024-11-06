#ifndef GLU_AST_TYPES_ENUMTY_HPP
#define GLU_AST_TYPES_ENUMTY_HPP

#include "SourceLocation.hpp"
#include "TypeBase.hpp"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <string>

namespace glu::types {

/// @brief EnumTy is a class that represents enumerations declared in code.
class EnumTy : public TypeBase {

    struct Case {
        std::string name;
        llvm::APInt value;
    };

    std::string _name;
    llvm::SmallVector<Case> _cases;
    SourceLocation _definitionLocation;

public:
    /// @brief Constructor for the EnumTy class.
    EnumTy(
        std::string name, llvm::SmallVector<Case> cases,
        SourceLocation definitionLocation
    )
        : TypeBase(TypeKind::EnumTyKind)
        , _name(name)
        , _cases(cases)
        , _definitionLocation(definitionLocation)
    {
    }

    std::string getName() const { return _name; }

    size_t getCaseCount() const { return _cases.size(); }
    Case const &getCase(size_t index) const { return _cases[index]; }

    SourceLocation getDefinitionLocation() const { return _definitionLocation; }

    std::optional<size_t> getCaseIndex(llvm::StringRef name) const
    {
        auto it = llvm::find_if(_cases, [&](Case const &field) {
            return field.name == name;
        });
        if (it == _cases.end())
            return std::nullopt;
        return std::distance(_cases.begin(), it);
    }

    /// @brief Static method to check if a type is a StructTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::EnumTyKind;
    }
};

} // namespace glu::types

#endif // GLU_AST_TYPES_ENUMTY_HPP
