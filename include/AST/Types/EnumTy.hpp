#ifndef GLU_AST_TYPES_ENUMTY_HPP
#define GLU_AST_TYPES_ENUMTY_HPP

#include "Basic/SourceLocation.hpp"
#include "Types/TypeBase.hpp"

#include <cassert>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>
#include <string>

namespace glu::types {

struct Case {
    llvm::StringRef name;
    llvm::APInt value;

    Case(llvm::StringRef const &n, llvm::APInt const &v) : name(n), value(v) { }
};

class EnumTy final : public TypeBase,
                     private llvm::TrailingObjects<EnumTy, Case> {
public:
    friend llvm::TrailingObjects<EnumTy, Case>;

private:
    llvm::StringRef _name;
    unsigned _numCases;
    SourceLocation _definitionLocation;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename llvm::TrailingObjects<EnumTy, Case>::OverloadToken<Case>) const
    {
        return _numCases;
    }

    EnumTy(std::string name, unsigned numCases, SourceLocation loc)
        : TypeBase(TypeKind::EnumTyKind)
        , _name(std::move(name))
        , _numCases(numCases)
        , _definitionLocation(loc)
    {
    }

public:
    EnumTy(
        std::string const &name, llvm::ArrayRef<Case> const &cases,
        SourceLocation loc
    )
        : EnumTy(name, cases.size(), loc)
    {
    }

    /// @brief Creates an EnumTy by allocating a memory block containing the
    /// object and its trailing objects for the cases.
    ///
    /// @param allocator The allocator (for example, llvm::BumpPtrAllocator)
    /// used for allocation.
    /// @param name The name of the enumeration.
    /// @param cases The cases to copy into the trailing storage.
    /// @param loc The definition location.
    /// @return A pointer to the new EnumTy.
    static EnumTy *create(
        llvm::BumpPtrAllocator &allocator, std::string const &name,
        llvm::ArrayRef<Case> const &cases, SourceLocation loc
    )
    {
        auto totalSize = sizeof(EnumTy) + cases.size() * sizeof(Case);
        void *mem = allocator.Allocate(totalSize, alignof(EnumTy));
        EnumTy *enumTy = new (mem) EnumTy(name, cases, loc);
        Case *dest = enumTy->template getTrailingObjects<Case>();
        for (size_t i = 0; i < cases.size(); ++i)
            new (&dest[i]) Case(cases[i].name, cases[i].value);
        return enumTy;
    }

    llvm::StringRef getName() const { return _name; }

    unsigned getCaseCount() const { return _numCases; }

    Case const &getCase(unsigned i) const
    {
        assert(i < _numCases && "Index out of bounds");
        return this->template getTrailingObjects<Case>()[i];
    }

    /// @brief Getter for the definition location of the enumeration.
    SourceLocation getDefinitionLocation() const { return _definitionLocation; }

    /// @brief Static method to check if a type is an EnumTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::EnumTyKind;
    }
};

} // namespace glu::types

#endif // GLU_AST_TYPES_ENUMTY_HPP
