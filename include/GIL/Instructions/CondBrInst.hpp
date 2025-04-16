#ifndef GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP

#include "TerminatorInst.hpp"
#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {
/// @class CondBrInst
/// @brief Represents a conditional branch instruction in the GIL.
///
/// This instruction is a control flow terminator that directs execution to one
/// of two possible successor basic blocks based on a given condition.
/// If the condition evaluates to true, execution proceeds to the "then" block;
/// otherwise, it proceeds to the "else" block. This instruction must always be
/// the last instruction in a basic block.
///
/// This instruction can also pass arguments to the destination basic blocks,
/// supporting phi-like functionality through basic block arguments.
class CondBrInst final
    : public TerminatorInst,
      private llvm::TrailingObjects<CondBrInst, Value, Value> {

    using TrailingArgs = llvm::TrailingObjects<CondBrInst, Value, Value>;
    friend TrailingArgs;

    Value condition;
    BasicBlock *thenBlock;
    BasicBlock *elseBlock;
    unsigned _thenArgsCount;
    unsigned _elseArgsCount;

    // Methods required by llvm::TrailingObjects to determine the number of
    // trailing objects
    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<Value>) const
    {
        return _thenArgsCount + _elseArgsCount;
    }

private:
    /// @brief Private constructor for the CondBrInst that takes trailing
    /// objects.
    ///
    /// @param condition The condition value that determines which branch to
    /// take.
    /// @param thenBlock The basic block to branch to if the condition is true.
    /// @param elseBlock The basic block to branch to if the condition is false.
    /// @param thenArgs The arguments to pass to the then block.
    /// @param elseArgs The arguments to pass to the else block.
    CondBrInst(
        Value condition, BasicBlock *thenBlock, BasicBlock *elseBlock,
        llvm::ArrayRef<Value> thenArgs, llvm::ArrayRef<Value> elseArgs
    )
        : TerminatorInst(InstKind::CondBrInstKind)
        , condition(condition)
        , thenBlock(thenBlock)
        , elseBlock(elseBlock)
        , _thenArgsCount(thenArgs.size())
        , _elseArgsCount(elseArgs.size())
    {
        // Ensure the number of arguments matches the number of parameters in
        // the destination blocks
        assert(
            thenBlock->getArgumentCount() == _thenArgsCount
            && "Number of arguments must match number of parameters in the "
               "then block"
        );
        assert(
            elseBlock->getArgumentCount() == _elseArgsCount
            && "Number of arguments must match number of parameters in the "
               "else block"
        );

        std::uninitialized_copy(
            thenArgs.begin(), thenArgs.end(), getThenArgsPtr()
        );
        std::uninitialized_copy(
            elseArgs.begin(), elseArgs.end(), getElseArgsPtr()
        );
    }

public:
    /// @brief Static factory method to create a CondBrInst without arguments.
    ///
    /// @param arena The memory arena to allocate from.
    /// @param condition The condition value that determines which branch to
    /// take.
    /// @param thenBlock The basic block to branch to if the condition is true.
    /// @param elseBlock The basic block to branch to if the condition is false.
    static CondBrInst *create(
        llvm::BumpPtrAllocator &arena, Value condition, BasicBlock *thenBlock,
        BasicBlock *elseBlock
    )
    {
        auto totalSize = totalSizeToAlloc<Value, Value>(0, 0);
        void *mem = arena.Allocate(totalSize, alignof(CondBrInst));

        return new (mem) CondBrInst(condition, thenBlock, elseBlock, {}, {});
    }

    /// @brief Static factory method to create a CondBrInst with arguments for
    /// both branches.
    ///
    /// @param arena The memory arena to allocate from.
    /// @param condition The condition value that determines which branch to
    /// take.
    /// @param thenBlock The basic block to branch to if the condition is true.
    /// @param elseBlock The basic block to branch to if the condition is false.
    /// @param thenArgs The arguments to pass to the then block.
    /// @param elseArgs The arguments to pass to the else block.
    static CondBrInst *create(
        llvm::BumpPtrAllocator &arena, Value condition, BasicBlock *thenBlock,
        BasicBlock *elseBlock, llvm::ArrayRef<Value> thenArgs,
        llvm::ArrayRef<Value> elseArgs
    )
    {
        auto totalSize
            = totalSizeToAlloc<Value, Value>(thenArgs.size(), elseArgs.size());
        void *mem = arena.Allocate(totalSize, alignof(CondBrInst));

        return new (mem)
            CondBrInst(condition, thenBlock, elseBlock, thenArgs, elseArgs);
    }

    // Helper methods to access the trailing objects
    Value *getThenArgsPtr() { return getTrailingObjects<Value>(); }
    Value const *getThenArgsPtr() const { return getTrailingObjects<Value>(); }

    Value *getElseArgsPtr()
    {
        return getTrailingObjects<Value>() + _thenArgsCount;
    }
    Value const *getElseArgsPtr() const
    {
        return getTrailingObjects<Value>() + _thenArgsCount;
    }

    Value getCondition() const { return condition; }
    BasicBlock *getThenBlock() const { return thenBlock; }
    BasicBlock *getElseBlock() const { return elseBlock; }

    llvm::ArrayRef<Value> getThenArgs() const
    {
        return llvm::ArrayRef<Value>(getThenArgsPtr(), _thenArgsCount);
    }

    llvm::ArrayRef<Value> getElseArgs() const
    {
        return llvm::ArrayRef<Value>(getElseArgsPtr(), _elseArgsCount);
    }

    bool hasBranchArgs() const
    {
        return _thenArgsCount > 0 || _elseArgsCount > 0;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CondBrInstKind;
    }

    size_t getOperandCount() const override
    {
        // 3 base operands (condition, thenBlock, elseBlock) + branch arguments
        return 3 + _thenArgsCount + _elseArgsCount;
    }

    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return condition;
        if (index == 1)
            return thenBlock;
        if (index == 2)
            return elseBlock;

        // Handle then arguments
        if (index < 3 + _thenArgsCount) {
            return getThenArgsPtr()[index - 3];
        }

        // Handle else arguments
        if (index < 3 + _thenArgsCount + _elseArgsCount) {
            return getElseArgsPtr()[index - 3 - _thenArgsCount];
        }

        assert(false && "Invalid operand index");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
