
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

enum class OperandKind {
    /// The operand is a value defined by an instruction, or an argument of a basic block. (%0, %1, etc.) (GILValue)
    ValueKind,
    /// The operand is a literal integer value. (42, -1, etc.)
    LiteralIntKind,
    /// The operand is a literal floating-point value. (3.14, -0.5, etc.)
    LiteralFloatKind,
    /// The operand is a literal string value. ("Hello, world!", etc.)
    LiteralStringKind,
    /// The operand is a reference to a function or global symbol. (@main, @printf, etc.)
    SymbolKind,
    /// The operand is a reference to a type. ($Int8, $Float, etc.) (GILType)
    TypeKind,
    /// The operand is a reference to a struct or enum member. (@MyStruct::field, @MyEnum::variant, etc.)
    MemberKind,
    /// The operand is a reference to a basic block / label. (entry, then, etc.)
    LabelKind,
};

class GILOperand {
    OperandKind kind;
    union {
        GILValue value;
        llvm::APInt literalInt;
        llvm::APFloat literalFloat;
        llvm::StringRef literalString;
        // GILFunction symbol;
        // GILType type;
        // GILMember member;
        GILBasicBlock *label;
    } data;

public:
    GILOperand(GILValue value) : kind(OperandKind::ValueKind) {
        data.value = value;
    }
    GILOperand(llvm::APInt literalInt) : kind(OperandKind::LiteralIntKind) {
        data.literalInt = literalInt;
    }
    GILOperand(llvm::APFloat literalFloat) : kind(OperandKind::LiteralFloatKind) {
        data.literalFloat = literalFloat;
    }
    GILOperand(llvm::StringRef literalString) : kind(OperandKind::LiteralStringKind) {
        data.literalString = literalString;
    }
    // GILOperand(GILFunction symbol) : kind(OperandKind::SymbolKind) {
    //     data.symbol = symbol;
    // }
    // GILOperand(GILType type) : kind(OperandKind::TypeKind) {
    //     data.type = type;
    // }
    // GILOperand(GILMember member) : kind(OperandKind::MemberKind) {
    //     data.member = member;
    // }
    GILOperand(GILBasicBlock *label) : kind(OperandKind::LabelKind) {
        data.label = label;
    }

    OperandKind getKind() { return kind; }

    GILValue getGILValue() {
        assert(kind == OperandKind::ValueKind && "Operand is not a value");
        return data.value;
    }
    llvm::APInt getLiteralInt() {
        assert(kind == OperandKind::LiteralIntKind && "Operand is not a literal integer");
        return data.literalInt;
    }
    llvm::APFloat getLiteralFloat() {
        assert(kind == OperandKind::LiteralFloatKind && "Operand is not a literal float");
        return data.literalFloat;
    }
    llvm::StringRef getLiteralString() {
        assert(kind == OperandKind::LiteralStringKind && "Operand is not a literal string");
        return data.literalString;
    }
    // GILFunction getSymbol() {
    //     assert(kind == OperandKind::SymbolKind && "Operand is not a symbol");
    //     return data.symbol;
    // }
    // GILType getType() {
    //     assert(kind == OperandKind::TypeKind && "Operand is not a type");
    //     return data.type;
    // }
    // GILMember getMember() {
    //     assert(kind == OperandKind::MemberKind && "Operand is not a member");
    //     return data.member;
    // }
    GILBasicBlock *getLabel() {
        assert(kind == OperandKind::LabelKind && "Operand is not a label");
        return data.label;
    }
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
    /// Returns the number of operands consumed by this instruction.
    virtual size_t getOperandCount() = 0;
    /// Returns the operand at the specified index. The index must be less than the
    /// value returned by getOperandCount().
    virtual GILOperand getOperand(size_t index) = 0;

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
 * which are instructions with exactly two operands (one type, one value) and one result.
 */
class ConversionInstBase : public InstBase {
protected:
    // GILType destType;
    GILValue operand;
public:
    // GILType getDestType() { return destType; }
    GILValue getOperand() { return operand; }

    size_t getResultCount() override { return 1; }
    size_t getOperandCount() override { return 2; }
    GILOperand getOperand(size_t index) override {
        switch (index) {
        case 0: // return getDestType();
        case 1: return getOperand();
        default: llvm_unreachable("Invalid operand index");
        }
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