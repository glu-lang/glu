#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include "Instructions/InstBase.hpp"
#include "Instructions/Terminator/TerminatorInst.hpp"
#include "Types/TypeBase.hpp"

#include <string>
#include <vector>

namespace glu::gil {

class Function;

/// @class BasicBlock
/// @brief Represents a basic block for instructions in the GIL (Glu
/// Intermediate Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil/#basic-blocks
class BasicBlock final
    : public llvm::ilist_node<BasicBlock, llvm::ilist_parent<Function>>,
      private llvm::TrailingObjects<BasicBlock, glu::gil::Type> {
    using NodeBase = llvm::ilist_node<BasicBlock, llvm::ilist_parent<Function>>;
    using TrailingArgs = llvm::TrailingObjects<BasicBlock, glu::gil::Type>;
    friend TrailingArgs;

public:
    using InstListType = llvm::iplist<InstBase, llvm::ilist_parent<BasicBlock>>;

private:
    friend llvm::ilist_traits<BasicBlock>;
    friend class Function; // Allow Function to set itself as the parent
                           // when added

    InstListType _instructions;
    llvm::StringRef _label;
    unsigned _argCount;

    size_t numTrailingObjects(
        typename TrailingArgs::OverloadToken<glu::types::TypeBase *>
    ) const
    {
        return _argCount;
    }

    BasicBlock(llvm::StringRef label, llvm::ArrayRef<gil::Type> args)
        : _label(label), _argCount(args.size())
    {
        std::uninitialized_copy(
            args.begin(), args.end(), getTrailingObjects<gil::Type>()
        );
    }

public:
    static BasicBlock *
    create(llvm::StringRef label, llvm::ArrayRef<gil::Type> args)
    {
        void *mem = ::operator new(totalSizeToAlloc<gil::Type>(args.size()));
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
    Function *getParent() const
    {
        return const_cast<BasicBlock *>(this)->NodeBase::getParent();
    }
    /// Set the parent function of this basic block
    void setParent(Function *parent) { this->NodeBase::setParent(parent); }

    Value getArgument(std::size_t index)
    {
        assert(index < _argCount && "Index out of bounds");
        return Value(this, index, getArgumentTypes()[index]);
    }

    std::size_t getArgumentCount() const { return _argCount; }

    llvm::ArrayRef<gil::Type> getArgumentTypes() const
    {
        return { getTrailingObjects<gil::Type>(), _argCount };
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
        block->setParent(getContainingFunction());
    }

    void removeNodeFromList(glu::gil::BasicBlock *block)
    {
        block->setParent(nullptr);
    }
};

} // end namespace llvm

#endif // GLU_GIL_BASICBLOCK_HPP
