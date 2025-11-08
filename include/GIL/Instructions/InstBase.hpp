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
#include "InstMacros.hpp"
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

class InstListBase : public ilist_base<false, glu::gil::BasicBlock> {
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
        using parent_ty = glu::gil::BasicBlock;
        using node_base_type
            = ilist_node_base<enable_sentinel_tracking, parent_ty>;
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
    using NodeBase = llvm::ilist_node<InstBase>;
    /// The source location of this instruction.
    SourceLocation _loc = SourceLocation::invalid;
    /// The kind of this instruction, used for LLVM-style RTTI.
    InstKind _kind;
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
    void setParent(BasicBlock *p) { this->NodeBase::setParent(p); }

    /// Returns the basic block that contains this instruction.
    BasicBlock *getParent() { return this->NodeBase::getParent(); }
    BasicBlock const *getParent() const
    {
        return const_cast<InstBase *>(this)->NodeBase::getParent();
    }

    /// @brief Removes this instruction from its parent basic block.
    void eraseFromParent();

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

#include "Conversions/ConversionInst.hpp"
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
        I->setParent(getContainingBlock());
    }

    void removeNodeFromList(glu::gil::InstBase *I) { I->setParent(nullptr); }

private:
    void createNode(glu::gil::InstBase const &);
};

} // end namespace llvm

#endif // GLU_GIL_INSTBASE_HPP
