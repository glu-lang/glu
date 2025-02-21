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
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace glu {
template <typename T> class InternedMemoryArena;
}

namespace glu::types {

struct Case {
    llvm::StringRef name;
    llvm::APInt value;
};

class EnumTy final : public TypeBase,
                     private llvm::TrailingObjects<EnumTy, Case> {
public:
    friend llvm::TrailingObjects<EnumTy, Case>;
    friend class InternedMemoryArena<TypeBase>;

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

    EnumTy(llvm::StringRef name, unsigned numCases, SourceLocation loc)
        : TypeBase(TypeKind::EnumTyKind)
        , _name(name)
        , _numCases(numCases)
        , _definitionLocation(loc)
    {
    }

    EnumTy(llvm::StringRef name, llvm::ArrayRef<Case> cases, SourceLocation loc)
        : EnumTy(name, cases.size(), loc)
    {
    }

public:
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
        llvm::BumpPtrAllocator &allocator, llvm::StringRef name,
        llvm::ArrayRef<Case> cases, SourceLocation loc
    )
    {
        auto totalSize = totalSizeToAlloc<Case>(cases.size());
        void *mem = allocator.Allocate(totalSize, alignof(EnumTy));
        EnumTy *enumTy = new (mem) EnumTy(name, cases, loc);
        std::uninitialized_copy(
            cases.begin(), cases.end(),
            enumTy->template getTrailingObjects<Case>()
        );
        return enumTy;
    }

    llvm::StringRef getName() const { return _name; }

    unsigned getCaseCount() const { return _numCases; }

    Case const &getCase(unsigned i) const
    {
        assert(i < _numCases && "Index out of bounds");
        return this->template getTrailingObjects<Case>()[i];
    }

    std::optional<size_t> getCaseIndex(llvm::StringRef name) const
    {
        for (size_t i = 0; i < _numCases; ++i) {
            if (getCase(i).name == name)
                return i;
        }
        return std::nullopt;
    }

    /// @brief Getter for the definition location of the enumeration.
    SourceLocation getDefinitionLocation() const { return _definitionLocation; }

    /// @brief Static method to check if a type is an EnumTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::EnumTyKind;
    }

    /// @brief Getter for the cases of the enum.
    /// @return Returns the cases of the enum.
    llvm::SmallVector<Case> getCases() const { return _cases; }
};

inline llvm::raw_ostream &
operator<<(llvm::raw_ostream &out, EnumTy::Case const &c)
{
    return out << c.name << " = " << c.value;
}

} // namespace glu::types

#endif // GLU_AST_TYPES_ENUMTY_HPP
