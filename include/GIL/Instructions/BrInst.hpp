#ifndef GLU_GIL_INSTRUCTIONS_BR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_BR_INST_HPP

#include "TerminatorInst.hpp"
#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {
/// @class BrInst
/// @brief Represents a br instruction in the GIL.
///
/// This instruction is a control flow terminator, meaning it marks the end of
/// execution in a function. It does not produce any results and must always be
/// the last instruction in a basic block.
///
/// This instruction can also pass arguments to the destination basic block,
/// supporting phi-like functionality through basic block arguments.
class BrInst final : public TerminatorInst,
                     private llvm::TrailingObjects<BrInst, Value> {

    using TrailingArgs = llvm::TrailingObjects<BrInst, Value>;
    friend TrailingArgs;

    BasicBlock *destination;
    unsigned _argsCount;

    // Methods required by llvm::TrailingObjects to determine the number of
    // trailing objects
    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<Value>) const
    {
        return _argsCount;
    }

private:
    /// @brief Private constructor for the BrInst that takes trailing objects.
    ///
    /// @param destination The basic block to branch to.
    /// @param args The arguments to pass to the destination block.
    BrInst(BasicBlock *destination, llvm::ArrayRef<Value> args)
        : TerminatorInst(InstKind::BrInstKind)
        , destination(destination)
        , _argsCount(args.size())
    {
        // Ensure the number of arguments matches the number of parameters in
        // the destination block
        assert(
            destination->getArgumentCount() == _argsCount
            && "Number of arguments must match number of parameters in the "
               "destination block"
        );

        // Use uninitialized_copy for raw memory
        std::uninitialized_copy(args.begin(), args.end(), getArgsPtr());
    }

public:
    /// @brief Static factory method to create a BrInst with optional arguments.
    ///
    /// @param arena The memory arena to allocate from.
    /// @param destination The basic block to branch to.
    /// @param args The arguments to pass to the destination block (empty by
    /// default).
    static BrInst *create(
        llvm::BumpPtrAllocator &arena, BasicBlock *destination,
        llvm::ArrayRef<Value> args = {}
    )
    {
        auto totalSize = totalSizeToAlloc<Value>(args.size());
        void *mem = arena.Allocate(totalSize, alignof(BrInst));

        return new (mem) BrInst(destination, args);
    }

    // Helper methods to access the trailing objects
    Value *getArgsPtr() { return getTrailingObjects<Value>(); }
    Value const *getArgsPtr() const { return getTrailingObjects<Value>(); }

    BasicBlock *getDestination() const { return destination; }

    llvm::ArrayRef<Value> getArgs() const
    {
        return llvm::ArrayRef<Value>(getArgsPtr(), _argsCount);
    }

    bool hasBranchArgs() const { return _argsCount > 0; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::BrInstKind;
    }

    size_t getOperandCount() const override
    {
        // 1 base operand (destination) + branch arguments
        return 1 + _argsCount;
    }

    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return destination;

        // Handle arguments
        if (index - 1 < _argsCount) {
            return getArgsPtr()[index - 1];
        }

        llvm_unreachable("Invalid operand index");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_BR_INST_HPP
