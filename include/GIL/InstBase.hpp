
#ifndef GLU_GIL_INSTBASE_HPP
#define GLU_GIL_INSTBASE_HPP

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/PointerUnion.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>

namespace glu::gil {

class InstBase;
class Function;
class BasicBlock {
    std::string label;
}; // FIXME: Placeholder
class Type { }; // FIXME: Placeholder
class Member { }; // FIXME: Placeholder

enum class InstKind {
#define GIL_INSTRUCTION(CLS, NAME, PARENT) CLS##Kind,
#define GIL_INSTRUCTION_SUPER(CLS, PARENT) CLS##FirstKind,
#define GIL_INSTRUCTION_SUPER_END(CLS) CLS##LastKind,
#include "InstKind.def"
#undef GIL_INSTRUCTION
#undef GIL_INSTRUCTION_SUPER
#undef GIL_INSTRUCTION_SUPER_END
};

/**
 * @class Value
 * @brief Represents a value reference in the GIL (Glu Intermediate Language).
 *
 * This class encapsulates a value that is either defined by an instruction or
 * is an argument of a basic block. It provides methods to retrieve the defining
 * instruction, the defining block, and the index of the value.
 *
 * Example GIL code:
 * @code
 * entry(%0, %1):
 *  %2 = add %0, %1
 *  return %2
 * @endcode
 *
 * In this example, %0 = Value(entry, 0), %1 = Value(entry, 1), and %2 =
 * Value(add, 0). %0 and %1 are basic block arguments, while %2 is a result of
 * the add instruction. The return instruction has no results. Values are given
 * indices in the order they are defined, at the time of printing the GIL code.
 * The indices are not stored anywhere in the GIL.
 */
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
};

enum class OperandKind {
    /// The operand is a value defined by an instruction, or an argument of a
    /// basic block. (%0, %1, etc.) (Value)
    ValueKind,
    /// The operand is a literal integer value. (42, -1, etc.)
    LiteralIntKind,
    /// The operand is a literal floating-point value. (3.14, -0.5, etc.)
    LiteralFloatKind,
    /// The operand is a literal string value. ("Hello, world!", etc.)
    LiteralStringKind,
    /// The operand is a reference to a function or global symbol. (@main,
    /// @printf, etc.)
    SymbolKind,
    /// The operand is a reference to a type. ($Int8, $Float, etc.) (Type)
    TypeKind,
    /// The operand is a reference to a struct or enum member.
    /// (@MyStruct::field, @MyEnum::variant, etc.)
    MemberKind,
    /// The operand is a reference to a basic block / label. (entry, then, etc.)
    LabelKind,
};

class Operand {
    OperandKind kind;
    union OperandData {
        Value value;
        llvm::APInt literalInt;
        llvm::APFloat literalFloat;
        llvm::StringRef literalString;
        Function *symbol;
        Type type;
        Member member;
        BasicBlock *label;

        OperandData(Value value) : value(value) { }
        OperandData(llvm::APInt literalInt) : literalInt(literalInt) { }
        OperandData(llvm::APFloat literalFloat) : literalFloat(literalFloat) { }
        OperandData(llvm::StringRef literalString)
            : literalString(literalString)
        {
        }
        OperandData(Function *symbol) : symbol(symbol) { }
        OperandData(Type type) : type(type) { }
        OperandData(Member member) : member(member) { }
        OperandData(BasicBlock *label) : label(label) { }
        ~OperandData() { } // Destruction is handled by ~Operand.
    } data;

public:
    Operand(Value value) : kind(OperandKind::ValueKind), data(std::move(value))
    {
    }
    Operand(llvm::APInt literalInt)
        : kind(OperandKind::LiteralIntKind), data(std::move(literalInt))
    {
    }
    Operand(llvm::APFloat literalFloat)
        : kind(OperandKind::LiteralFloatKind), data(std::move(literalFloat))
    {
    }
    Operand(llvm::StringRef literalString)
        : kind(OperandKind::LiteralStringKind), data(std::move(literalString))
    {
    }
    Operand(Function *symbol) : kind(OperandKind::SymbolKind), data(symbol) { }
    Operand(Type type) : kind(OperandKind::TypeKind), data(std::move(type)) { }
    Operand(Member member)
        : kind(OperandKind::MemberKind), data(std::move(member))
    {
    }
    Operand(BasicBlock *label) : kind(OperandKind::LabelKind), data(label) { }
    ~Operand()
    {
        switch (kind) {
        case OperandKind::ValueKind: data.value.~Value(); break;
        case OperandKind::LiteralIntKind: data.literalInt.~APInt(); break;
        case OperandKind::LiteralFloatKind: data.literalFloat.~APFloat(); break;
        case OperandKind::LiteralStringKind:
            data.literalString.~StringRef();
            break;
        case OperandKind::SymbolKind: break;
        case OperandKind::TypeKind: data.type.~Type(); break;
        case OperandKind::MemberKind: data.member.~Member(); break;
        case OperandKind::LabelKind: break;
        }
    }

    OperandKind getKind() const { return kind; }

    Value getValue() const
    {
        assert(kind == OperandKind::ValueKind && "Operand is not a value");
        return data.value;
    }
    llvm::APInt getLiteralInt() const
    {
        assert(
            kind == OperandKind::LiteralIntKind
            && "Operand is not a literal integer"
        );
        return data.literalInt;
    }
    llvm::APFloat getLiteralFloat() const
    {
        assert(
            kind == OperandKind::LiteralFloatKind
            && "Operand is not a literal float"
        );
        return data.literalFloat;
    }
    llvm::StringRef getLiteralString() const
    {
        assert(
            kind == OperandKind::LiteralStringKind
            && "Operand is not a literal string"
        );
        return data.literalString;
    }
    Function *getSymbol() const
    {
        assert(kind == OperandKind::SymbolKind && "Operand is not a symbol");
        return data.symbol;
    }
    Type getType() const
    {
        assert(kind == OperandKind::TypeKind && "Operand is not a type");
        return data.type;
    }
    Member getMember() const
    {
        assert(kind == OperandKind::MemberKind && "Operand is not a member");
        return data.member;
    }
    BasicBlock *getLabel() const
    {
        assert(kind == OperandKind::LabelKind && "Operand is not a label");
        return data.label;
    }
};

class ConversionInstBase;

/**
 * @class InstBase
 * @brief Represents the base class for instructions in the GIL (Glu
 * Intermediate Language).
 *
 * This class provides the basic interface for all instructions, including
 * methods to get the kind of instruction, its operands, and its results. It
 * also maintains a reference to the basic block that contains this instruction.
 *
 * @note This is an abstract class and cannot be instantiated directly.
 */
class InstBase {
    /// The kind of this instruction, used for LLVM-style RTTI.
    InstKind _kind;
    /// The basic block that contains this instruction.
    BasicBlock *parent = nullptr;
    friend class BasicBlock; // Allow BasicBlock to set itself as the parent
                             // when added.
public:
    InstBase(InstKind kind) : _kind(kind) { }
    virtual ~InstBase() = default;
    InstBase(InstBase const &) = delete;
    InstBase &operator=(InstBase const &) = delete;
    InstBase(InstBase &&) = delete;
    InstBase &operator=(InstBase &&) = delete;

    // These methods must be implemented by derived classes.

    /// Returns the number of results produced by this instruction. For
    /// terminator instructions, this is always 0. For other instructions, this
    /// is usually 1.
    virtual size_t getResultCount() const = 0;
    /// Returns the number of operands consumed by this instruction.
    virtual size_t getOperandCount() const = 0;
    /// Returns the operand at the specified index. The index must be less than
    /// the value returned by getOperandCount().
    virtual Operand getOperand(size_t index) const = 0;
    /// Returns the type of the result at the specified index. The index must be
    /// less than the value returned by getResultCount().
    virtual Type getResultType(size_t index) const = 0;

    Value getResult(size_t index)
    {
        assert(index < getResultCount() && "Result index out of range");
        return Value(this, index, getResultType(index));
    }

    BasicBlock *getParent() const { return parent; }
    llvm::StringRef getInstName() const;

    InstKind getKind() const { return _kind; }

    bool isConversion() { return llvm::isa<ConversionInstBase>(this); }
};

/**
 * @class ConversionInstBase
 * @brief A class representing a conversion instruction in the GIL.
 *
 * This class inherits from InstBase and provides functionality specific to
 * conversion instructions, which are instructions with exactly two operands
 * (one type, one value) and one result.
 */
class ConversionInstBase : public InstBase {
protected:
    Type destType;
    Value operand;

public:
    Type getDestType() const { return destType; }
    Value getOperand() const { return operand; }

    size_t getResultCount() const override { return 1; }
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return destType;
    }
    size_t getOperandCount() const override { return 2; }
    Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return getDestType();
        case 1: return getOperand();
        default: llvm_unreachable("Invalid operand index");
        }
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::ConversionInstFirstKind
            && inst->getKind() <= InstKind::ConversionInstLastKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTBASE_HPP
