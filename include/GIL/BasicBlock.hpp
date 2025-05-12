#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include "Instructions/InstBase.hpp"
#include "Types/TypeBase.hpp"

#include <string>
#include <vector>

namespace llvm::ilist_detail {
class BasicBlockListBase : public ilist_base<false> {
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

template <> struct compute_node_options<glu::gil::BasicBlock> {
    struct type {
        using value_type = glu::gil::BasicBlock;
        using pointer = value_type *;
        using reference = value_type &;
        using const_pointer = value_type const *;
        using const_reference = value_type const &;

        static bool const enable_sentinel_tracking = false;
        static bool const is_sentinel_tracking_explicit = false;
        static bool const has_iterator_bits = false;
        using tag = void;
        using node_base_type = ilist_node_base<enable_sentinel_tracking>;
        using list_base_type = BasicBlockListBase;
    };
};

} // end namespace llvm::ilist_detail

namespace glu::gil {

class Function;

/// @class BasicBlock
/// @brief Represents a basic block for instructions in the GIL (Glu
/// Intermediate Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil/#basic-blocks
class BasicBlock final
    : public llvm::ilist_node<BasicBlock>,
      private llvm::TrailingObjects<BasicBlock, glu::types::TypeBase *> {
    using TrailingArgs
        = llvm::TrailingObjects<BasicBlock, glu::types::TypeBase *>;
    friend TrailingArgs;

public:
    using InstListType = llvm::iplist<InstBase>;

private:
    /// The parent function of this basic block
    Function *_parent = nullptr;
    friend llvm::ilist_traits<BasicBlock>;
    friend class Function; // Allow Function to set itself as the parent
                           // when added

    InstListType _instructions;
    llvm::StringRef _label;
    unsigned _argCount;

    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<
                              glu::types::TypeBase *>) const
    {
        return _argCount;
    }

    BasicBlock(
        llvm::StringRef label, llvm::ArrayRef<glu::types::TypeBase *> args
    )
        : _label(label), _argCount(args.size())
    {
        std::uninitialized_copy(
            args.begin(), args.end(),
            getTrailingObjects<glu::types::TypeBase *>()
        );
    }

public:
    static BasicBlock *create(
        llvm::BumpPtrAllocator &allocator, llvm::StringRef label,
        llvm::ArrayRef<glu::types::TypeBase *> args
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<glu::types::TypeBase *>(args.size()),
            alignof(BasicBlock)
        );
        return new (mem) BasicBlock(label, args);
    }

    InstListType &getInstructions() { return _instructions; }

    std::size_t getInstructionCount() const { return _instructions.size(); }

    InstBase *popFirstInstruction();

    void addInstructionAtEnd(InstBase *inst) { _instructions.push_back(inst); }

    void addInstructionAtStart(InstBase *inst)
    {
        _instructions.push_front(inst);
    }

    void addInstructionAt(InstBase *inst, InstListType::iterator it)
    {
        _instructions.insert(it, inst);
    }

    /// Adds an instruction before the specified instruction.
    /// If before is nullptr, the instruction is added at the end (before no
    /// instruction).
    void addInstructionBefore(InstBase *inst, InstBase *before);
    /// Adds an instruction after the specified instruction.
    /// If after is nullptr, the instruction is added at the start (after no
    /// instruction).
    void addInstructionAfter(InstBase *inst, InstBase *after);
    /// Replaces the old instruction with the new instruction.
    void replaceInstruction(InstBase *oldInst, InstBase *newInst);
    /// Removes the specified instruction from the basic block.
    void removeInstruction(InstBase *inst);

    // defined to be used by ilist
    static InstListType BasicBlock::*getSublistAccess(InstBase *)
    {
        return &BasicBlock::_instructions;
    }

    TerminatorInst *getTerminator();
    void setTerminator(TerminatorInst *terminator);

    void setLabel(llvm::StringRef label) { _label = label; }
    llvm::StringRef const &getLabel() const { return _label; }

    /// Returns the parent function of this basic block
    Function *getParent() const { return _parent; }
    /// Set the parent function of this basic block
    void setParent(Function *parent) { _parent = parent; }

    Value getArgument(std::size_t index)
    {
        assert(index < _argCount && "Index out of bounds");
        return Value(this, index, {}); // _arguments[index]
    }

    std::size_t getArgumentCount() const { return _argCount; }

    llvm::ArrayRef<glu::types::TypeBase *> getArgumentTypes() const
    {
        return { getTrailingObjects<glu::types::TypeBase *>(), _argCount };
    }
};

} // end namespace glu::gil

///===----------------------------------------------------------------------===//
/// ilist_traits for BasicBlock
///===----------------------------------------------------------------------===//
namespace llvm {

template <>
struct ilist_traits<glu::gil::BasicBlock>
    : public ilist_node_traits<glu::gil::BasicBlock> {
private:
    glu::gil::Function *getContainingFunction();

public:
    void addNodeToList(glu::gil::BasicBlock *block)
    {
        block->_parent = getContainingFunction();
    }

    // Disable automatic deletion of BasicBlocks, since they're allocated with BumpPtrAllocator
    void deleteNode(glu::gil::BasicBlock *) { /* No-op: don't delete BasicBlocks */ }

private:
    void createNode(glu::gil::BasicBlock const &);
};

} // end namespace llvm

#endif // GLU_GIL_BASICBLOCK_HPP
