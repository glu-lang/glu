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

    GLU_GIL_GEN_OPERAND(Condition, Value, _condition)
    GLU_GIL_GEN_OPERAND(ThenBlock, BasicBlock *, _thenBlock)
    GLU_GIL_GEN_OPERAND(ElseBlock, BasicBlock *, _elseBlock)
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
        , _condition(condition)
        , _thenBlock(thenBlock)
        , _elseBlock(elseBlock)
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
    /// @brief Static factory method to create a CondBrInst with optional
    /// arguments.
    ///
    /// @param condition The condition value that determines which branch to
    /// take.
    /// @param thenBlock The basic block to branch to if the condition is true.
    /// @param elseBlock The basic block to branch to if the condition is false.
    /// @param thenArgs The arguments to pass to the then block (empty by
    /// default).
    /// @param elseArgs The arguments to pass to the else block (empty by
    /// default).
    static CondBrInst *create(
        Value condition, BasicBlock *thenBlock, BasicBlock *elseBlock,
        llvm::ArrayRef<Value> thenArgs = {}, llvm::ArrayRef<Value> elseArgs = {}
    )
    {
        auto totalSize
            = totalSizeToAlloc<Value, Value>(thenArgs.size(), elseArgs.size());
        void *mem = ::operator new(totalSize);

        return new (mem)
            CondBrInst(condition, thenBlock, elseBlock, thenArgs, elseArgs);
    }

    // Custom delete operator for TrailingObjects
    void operator delete(void *ptr) { ::operator delete(ptr); }

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
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
