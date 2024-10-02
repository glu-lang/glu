
#ifndef GLU_GIL_INSTBASE_HPP
#define GLU_GIL_INSTBASE_HPP

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/PointerUnion.h>

namespace glu::gil {

class GILValue;
class GILBasicBlock;

enum class InstKind {
    #define GIL_INSTRUCTION(CLS, NAME) CLS ## Kind,
    #include "InstKind.def"
    #undef GIL_INSTRUCTION
};

/**
 * @class InstBase
 * @brief Represents the base class for instructions in the GIL (Glu Intermediate Language).
 *
 * This class provides the basic interface for all instructions, including methods to
 * get the kind of instruction, its operands, and its results. It also maintains a reference
 * to the basic block that contains this instruction.
 *
 * @note This is an abstract class and cannot be instantiated directly.
 */
class InstBase {
    /// The basic block that contains this instruction.
    GILBasicBlock *parent = nullptr;
    friend class GILBasicBlock; // Allow GILBasicBlock to set itself as the parent when added.
public:
    InstBase() {}
    virtual ~InstBase() = default;
    InstBase(const InstBase &) = delete;
    InstBase &operator=(const InstBase &) = delete;
    InstBase(InstBase &&) = delete;
    InstBase &operator=(InstBase &&) = delete;

    // These methods must be implemented by derived classes.

    /// Returns the kind of this instruction, which is a value from the InstKind enumeration.
    virtual InstKind getKind() = 0;
    /// Returns the number of results produced by this instruction. For terminator
    /// instructions, this is always 0. For other instructions, this is usually 1.
    virtual size_t getResultCount() = 0;
    /// Returns the number of operands consumed by this instruction. For literals and
    /// other instructions that do not consume operands, this is always 0.
    virtual size_t getOperandCount() = 0;
    /// Returns the operand at the specified index. The index must be less than the
    /// value returned by getOperandCount().
    virtual GILValue getOperand(size_t index) = 0;

    GILValue getResult(size_t index) {
        assert(index < getResultCount() && "Result index out of range");
        return GILValue(this, index);
    }
    // virtual GILType getResultType(size_t index) = 0;

    GILBasicBlock *getParent() { return parent; }
    llvm::StringRef getInstName();
};


/**
 * @class ConversionInstBase
 * @brief A class representing a conversion instruction in the GIL.
 * 
 * This class inherits from InstBase and provides functionality specific to conversion instructions,
 * which are instructions with exactly one operand and one result.
 */
class ConversionInstBase : public InstBase {
    GILValue operand;
public:
    GILValue getOperand() { return operand; }

    size_t getResultCount() override { return 1; }
    size_t getOperandCount() override { return 1; }
    GILValue getOperand(size_t index) override {
        assert(index == 0 && "Invalid operand index");
        return getOperand();
    }
};

/**
 * @class GILValue
 * @brief Represents a value reference in the GIL (Glu Intermediate Language).
 *
 * This class encapsulates a value that is either defined by an instruction or is an argument of a basic block.
 * It provides methods to retrieve the defining instruction, the defining block, and the index of the value.
 * 
 * Example GIL code:
 * @code
 * entry(%0, %1):
 *  %2 = add %0, %1
 *  return %2
 * @endcode
 * 
 * In this example, %0 = GILValue(entry, 0), %1 = GILValue(entry, 1), and %2 = GILValue(add, 0). %0 and %1 are
 * basic block arguments, while %2 is a result of the add instruction. The return instruction has no results.
 * Values are given indices in the order they are defined, at the time of printing the GIL code. The indices are
 * not stored anywhere in the GIL.
 */
class GILValue {
    llvm::PointerUnion<InstBase *, GILBasicBlock *> value;
    unsigned index;
    // GILType type;
    friend class InstBase;
public:
    /// Returns the instruction that defines this value, or nullptr if it is a basic block argument.
    InstBase *getDefiningInstruction() {
        return value.dyn_cast<InstBase *>();
    }
    /// Returns the basic block in which this value is defined.
    GILBasicBlock *getDefiningBlock() {
        if (auto block = value.dyn_cast<GILBasicBlock *>()) {
            return block;
        }
        return value.get<InstBase *>()->getParent();
    }
    /// Returns the index of this value in the list of results of the defining instruction.
    unsigned getIndex() { return index; }

};

} // namespace glu::gil

#endif // GLU_GIL_INSTBASE_HPP