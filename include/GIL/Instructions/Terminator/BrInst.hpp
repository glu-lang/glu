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
    GLU_GIL_GEN_OPERAND(Destination, BasicBlock *, _destination)
    GLU_GIL_GEN_OPERAND_LIST_TRAILING_OBJECTS(BrInst, _argsCount, Value, Args)

private:
    /// @brief Private constructor for the BrInst that takes trailing objects.
    ///
    /// @param destination The basic block to branch to.
    /// @param args The arguments to pass to the destination block.
    BrInst(BasicBlock *destination, llvm::ArrayRef<Value> args)
        : TerminatorInst(InstKind::BrInstKind), _destination(destination)
    {
        // Ensure the number of arguments matches the number of parameters in
        // the destination block
        assert(
            destination->getArgumentCount() == args.size()
            && "Number of arguments must match number of parameters in the "
               "destination block"
        );

        initArgs(args);
    }

public:
    /// @brief Static factory method to create a BrInst with optional arguments.
    ///
    /// @param destination The basic block to branch to.
    /// @param args The arguments to pass to the destination block (empty by
    /// default).
    static BrInst *
    create(BasicBlock *destination, llvm::ArrayRef<Value> args = {})
    {
        auto totalSize = totalSizeToAlloc<Value>(args.size());
        void *mem = ::operator new(totalSize);

        return new (mem) BrInst(destination, args);
    }

    // Custom delete operator for TrailingObjects
    void operator delete(void *ptr) { ::operator delete(ptr); }

    bool hasBranchArgs() const { return _argsCount > 0; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::BrInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_BR_INST_HPP
