#ifndef GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP

#include "TerminatorInst.hpp"
#include <llvm/ADT/SmallVector.h>

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
class CondBrInst : public TerminatorInst {
    Value condition;
    BasicBlock *thenBlock;
    BasicBlock *elseBlock;
    llvm::SmallVector<Value, 2> thenArgs;
    llvm::SmallVector<Value, 2> elseArgs;

public:
    /// @brief Constructs a CondBrInst without arguments.
    ///
    /// @param condition The condition value that determines which branch to
    /// take.
    /// @param thenBlock The basic block to branch to if the condition is true.
    /// @param elseBlock The basic block to branch to if the condition is false.
    CondBrInst(Value condition, BasicBlock *thenBlock, BasicBlock *elseBlock)
        : TerminatorInst(InstKind::CondBrInstKind)
        , condition(condition)
        , thenBlock(thenBlock)
        , elseBlock(elseBlock)
    {
    }

    /// @brief Constructs a CondBrInst with arguments for the then and else
    /// branches.
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
        , thenArgs(thenArgs.begin(), thenArgs.end())
        , elseArgs(elseArgs.begin(), elseArgs.end())
    {
        // Ensure the number of arguments matches the number of parameters in
        // the destination blocks
        assert(
            thenBlock->getArgumentCount() == thenArgs.size()
            && "Number of arguments must match number of parameters in the "
               "then block"
        );
        assert(
            elseBlock->getArgumentCount() == elseArgs.size()
            && "Number of arguments must match number of parameters in the "
               "else block"
        );
    }

    Value getCondition() const { return condition; }
    BasicBlock *getThenBlock() const { return thenBlock; }
    BasicBlock *getElseBlock() const { return elseBlock; }
    llvm::ArrayRef<Value> getThenArgs() const { return thenArgs; }
    llvm::ArrayRef<Value> getElseArgs() const { return elseArgs; }

    bool hasBranchArgs() const
    {
        return !thenArgs.empty() || !elseArgs.empty();
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CondBrInstKind;
    }

    size_t getOperandCount() const override
    {
        // 3 base operands (condition, thenBlock, elseBlock) + branch arguments
        return 3 + thenArgs.size() + elseArgs.size();
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
        if (index < 3 + thenArgs.size()) {
            return thenArgs[index - 3];
        }

        // Handle else arguments
        if (index < 3 + thenArgs.size() + elseArgs.size()) {
            return elseArgs[index - 3 - thenArgs.size()];
        }

        assert(false && "Invalid operand index");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
