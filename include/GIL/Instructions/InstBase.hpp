#ifndef GLU_GIL_INSTBASE_HPP
#define GLU_GIL_INSTBASE_HPP

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ilist.h>
#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/Casting.h>

#include "Basic/SourceLocation.hpp"
#include "Global.hpp"
#include "Member.hpp"
#include "Type.hpp"
#include "Value.hpp"

// Forward declarations
namespace glu::gil {
class Function;
class BasicBlock;

class InstBase;
#define GIL_INSTRUCTION(CLS, STR, PARENT) class CLS;
#define GIL_INSTRUCTION_SUPER(CLS, PARENT) class CLS;
#include "InstKind.def"
} // end namespace glu::gil

namespace llvm::ilist_detail {

class InstListBase : public ilist_base<false> {
public:
    template <class T> static void remove(T &N) { removeImpl(N); }

    template <class T> static void insertBefore(T &Next, T &N)
    {
        insertBeforeImpl(Next, N);
    }

    template <class T> static void transferBefore(T &Next, T &First, T &Last)
    {
        transferBeforeImpl(Next, First, Last);
    }
};

template <> struct compute_node_options<glu::gil::InstBase> {
    struct type {
        using value_type = glu::gil::InstBase;
        using pointer = value_type *;
        using reference = value_type &;
        using const_pointer = value_type const *;
        using const_reference = value_type const &;

        static bool const enable_sentinel_tracking = false;
        static bool const is_sentinel_tracking_explicit = false;
        static bool const has_iterator_bits = false;
        using tag = void;
        using node_base_type = ilist_node_base<enable_sentinel_tracking>;
        using list_base_type = InstListBase;
    };
};

} // end namespace llvm::ilist_detail

namespace glu::gil {

enum class InstKind {
#define GIL_INSTRUCTION(CLS, NAME, PARENT) CLS##Kind,
#define GIL_INSTRUCTION_SUPER(CLS, PARENT) CLS##FirstKind,
#define GIL_INSTRUCTION_SUPER_END(CLS) CLS##LastKind,
#include "InstKind.def"
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
    /// The operand is a reference to a function symbol. (@main,
    /// @printf, etc.)
    SymbolKind,
    /// The operand is a reference to a global variable symbol. (@my_global,
    /// etc.)
    GlobalKind,
    /// The operand is a reference to a type. ($Int8, $Float, etc.) (Type)
    TypeKind,
    /// The operand is a reference to a struct or enum member.
    /// (@MyStruct::field, @MyEnum::variant, etc.)
    MemberKind,
    /// The operand is a reference to a basic block / label. (entry, then, etc.)
    LabelKind,
};

/// The `Operand` class represents an operand of an instruction.
/// It can have different kinds of data, such as values, literals, symbols,
/// types, members, and labels. Usually, kinds of operands within an instruction
/// don't change. A few instructions can have different kinds of operands at one
/// index, but this is rare.
///
/// The following types of data can be stored in an `Operand`:
/// - `Value`: Represents a value (%0, %1, etc.).
/// - `llvm::APInt`: Represents a literal integer (42, -1, etc.).
/// - `llvm::APFloat`: Represents a literal float (3.14, -0.5, etc.).
/// - `llvm::StringRef`: Represents a literal string ("Hello, world!", etc.).
/// - `Function *`: Represents a function (@main, @printf, etc.).
/// - `Type`: Represents a type ($Int8, $Float, etc.).
/// - `Member`: Represents a member (@MyStruct::field, @MyEnum::variant, etc.).
/// - `BasicBlock *`: Represents a label (entry, then, etc.).
///
/// The class provides getter methods for each type of data, which assert that
/// the operand is of the correct kind.
class Operand {
    OperandKind kind;
    union OperandData {
        Value value;
        llvm::APInt literalInt;
        llvm::APFloat literalFloat;
        llvm::StringRef literalString;
        Function *symbol;
        Global *global;
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
        OperandData(Global *global) : global(global) { }
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
    Operand(Global *global) : kind(OperandKind::GlobalKind), data(global) { }
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
        case OperandKind::GlobalKind: break;
        case OperandKind::TypeKind: data.type.~Type(); break;
        case OperandKind::MemberKind: data.member.~Member(); break;
        case OperandKind::LabelKind: break;
        }
    }

    OperandKind getKind() const { return kind; }

    /// Returns this operand as a value. This must be a ValueKind operand.
    Value getValue() const
    {
        assert(kind == OperandKind::ValueKind && "Operand is not a value");
        return data.value;
    }

    /// Returns this operand as an arbitrary precision integer. This must be a
    /// LiteralIntKind operand.
    llvm::APInt getLiteralInt() const
    {
        assert(
            kind == OperandKind::LiteralIntKind
            && "Operand is not a literal integer"
        );
        return data.literalInt;
    }

    /// Returns this operand as an arbitrary precision float. This must be a
    /// LiteralFloatKind operand.
    llvm::APFloat getLiteralFloat() const
    {
        assert(
            kind == OperandKind::LiteralFloatKind
            && "Operand is not a literal float"
        );
        return data.literalFloat;
    }

    /// Returns this operand as a string reference. This must be a
    /// LiteralStringKind operand.
    llvm::StringRef getLiteralString() const
    {
        assert(
            kind == OperandKind::LiteralStringKind
            && "Operand is not a literal string"
        );
        return data.literalString;
    }

    /// Returns this operand as a function symbol. This must be a SymbolKind
    /// operand.
    Function *getSymbol() const
    {
        assert(kind == OperandKind::SymbolKind && "Operand is not a symbol");
        return data.symbol;
    }

    /// Returns this operand as a global variable symbol. This must be a
    /// GlobalKind operand.
    Global *getGlobal() const
    {
        assert(kind == OperandKind::GlobalKind && "Operand is not a global");
        return data.global;
    }

    /// Returns this operand as a type. This must be a TypeKind operand.
    Type getType() const
    {
        assert(kind == OperandKind::TypeKind && "Operand is not a type");
        return data.type;
    }

    /// Returns this operand as a member. This must be a MemberKind operand.
    Member getMember() const
    {
        assert(kind == OperandKind::MemberKind && "Operand is not a member");
        return data.member;
    }

    /// Returns this operand as a basic block / label. This must be a LabelKind
    /// operand.
    BasicBlock *getLabel() const
    {
        assert(kind == OperandKind::LabelKind && "Operand is not a label");
        return data.label;
    }
};

class ConversionInst;

/// @class InstBase
/// @brief Represents the base class for instructions in the GIL (Glu
/// Intermediate Language).
///
/// This class provides the basic interface for all instructions, including
/// methods to get the kind of instruction, its operands, and its results. It
/// also maintains a reference to the basic block that contains this
/// instruction.
///
/// @note This is an abstract class and cannot be instantiated directly.
class InstBase : public llvm::ilist_node<InstBase> {
    /// The source location of this instruction.
    SourceLocation _loc = SourceLocation::invalid;
    /// The kind of this instruction, used for LLVM-style RTTI.
    InstKind _kind;
    /// The basic block that contains this instruction.
    BasicBlock *parent = nullptr;
    friend llvm::ilist_traits<InstBase>;
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

    /// Returns the nth result of this instruction. The index must be in range.
    Value getResult(size_t index)
    {
        assert(index < getResultCount() && "Result index out of range");
        return Value(this, index, getResultType(index));
    }

    /// Set the parent basic block of this instruction. (Internal use only by
    /// ilist)
    void setParent(BasicBlock *p) { parent = p; }

    /// Returns the const basic block that contains this instruction.
    BasicBlock *getParent() const { return parent; }

    /// Returns the basic block that contains this instruction.
    inline BasicBlock *getParent() { return parent; }

    /// Returns the name of this instruction.
    llvm::StringRef getInstName() const;

    /// Returns the kind of this instruction.
    InstKind getKind() const { return _kind; }

    /// Returns true if this instruction is a terminator instruction.
    bool isTerminator() { return llvm::isa<TerminatorInst>(this); }
    /// Returns true if this instruction is a conversion instruction.
    bool isConversion() { return llvm::isa<ConversionInst>(this); }

    /// @brief Set the source location of this instruction.
    /// @param loc The source location to set.
    void setLocation(SourceLocation loc) { _loc = loc; }

    /// @brief Get the source location of this instruction.
    /// @return The source location of this instruction.
    SourceLocation getLocation() const { return _loc; }

    /// @brief Print a human-readable representation of this instruction to
    /// standard output, for debugging purposes.
    void print();
};

} // end namespace glu::gil

#include "ConversionInst.hpp"
#include "TerminatorInst.hpp"

///===----------------------------------------------------------------------===//
/// ilist_traits for InstBase
///===----------------------------------------------------------------------===//
namespace llvm {

template <>
struct ilist_traits<glu::gil::InstBase>
    : public ilist_node_traits<glu::gil::InstBase> {
private:
    glu::gil::BasicBlock *getContainingBlock();

public:
    void addNodeToList(glu::gil::InstBase *I)
    {
        I->parent = getContainingBlock();
    }

private:
    void createNode(glu::gil::InstBase const &);
};

} // end namespace llvm

#endif // GLU_GIL_INSTBASE_HPP
