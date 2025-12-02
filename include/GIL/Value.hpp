#ifndef GLU_GIL_INSTRUCTIONS_VALUE_HPP
#define GLU_GIL_INSTRUCTIONS_VALUE_HPP

#include "Types/TypeBase.hpp"

#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/PointerUnion.h>

namespace glu::gil {

/// Type alias for GIL types - GIL uses AST types directly
using Type = types::Ty;

class InstBase;
class BasicBlock;

/// @class Value
/// @brief Represents a value reference in the GIL (Glu Intermediate Language).
///
/// This class encapsulates a value that is either defined by an instruction or
/// is an argument of a basic block. It provides methods to retrieve the
/// defining instruction, the defining block, and the index of the value.
///
/// Example GIL code:
/// @code
/// entry(%0, %1):
///  %2 = add %0, %1
///  return %2
/// @endcode
///
/// In this example, %0 = Value(entry, 0), %1 = Value(entry, 1), and %2 =
/// Value(add, 0). %0 and %1 are basic block arguments, while %2 is a result of
/// the add instruction. The return instruction has no results. Values are given
/// indices in the order they are defined, at the time of printing the GIL code.
/// The indices are not stored anywhere in the GIL.
class Value {
    llvm::PointerUnion<InstBase *, BasicBlock *> value;
    unsigned index;
    Type type;
    friend class InstBase;
    friend class BasicBlock;
    Value(InstBase *inst, unsigned index, Type type)
        : value(inst), index(index), type(type)
    {
    }
    Value(BasicBlock *block, unsigned index, Type type)
        : value(block), index(index), type(type)
    {
    }

public:
    /// Returns the instruction that defines this value, or nullptr if it is a
    /// basic block argument.
    InstBase *getDefiningInstruction() const
    {
        return value.dyn_cast<InstBase *>();
    }

    /// Returns the basic block in which this value is defined.
    BasicBlock *getDefiningBlock() const;

    /// Returns the index of this value in the list of results of the defining
    /// instruction.
    unsigned getIndex() const { return index; }

    /// Returns the type of this value.
    Type getType() const { return type; }

    /// @brief Replaces all uses of this value with a new value.
    /// @param newValue The new value that replaces this one.
    void replaceAllUsesWith(Value newValue);

    bool operator==(Value const &other) const
    {
        return value == other.value && index == other.index;
    }

    bool operator!=(Value const &other) const { return !(*this == other); }

    static Value getEmptyKey()
    {
        return Value(static_cast<InstBase *>(nullptr), 0, nullptr);
    }

    static Value getTombstoneKey()
    {
        return Value(static_cast<InstBase *>(nullptr), -1, nullptr);
    }
};

} // namespace glu::gil

namespace llvm {

// support for Value keys in DenseMap
template <> struct DenseMapInfo<glu::gil::Value> {
    static inline glu::gil::Value getEmptyKey()
    {
        return glu::gil::Value::getEmptyKey();
    }

    static inline glu::gil::Value getTombstoneKey()
    {
        return glu::gil::Value::getTombstoneKey();
    }

    static unsigned getHashValue(glu::gil::Value const &val)
    {
        return DenseMapInfo<std::pair<glu::gil::InstBase *, unsigned>>::
            getHashValue(
                std::make_pair(val.getDefiningInstruction(), val.getIndex())
            );
    }

    static bool isEqual(glu::gil::Value const &lhs, glu::gil::Value const &rhs)
    {
        return lhs == rhs;
    }
};

} // namespace llvm

#endif // GLU_GIL_INSTRUCTIONS_VALUE_HPP
