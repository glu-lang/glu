#ifndef GLU_GIL_INSTRUCTIONS_PTR_OFFSET_INST_HPP
#define GLU_GIL_INSTRUCTIONS_PTR_OFFSET_INST_HPP

#include "AggregateInst.hpp"

namespace glu::gil {

/// @class PtrOffsetInst
/// @brief Represents a pointer offset instruction in GIL.
///
/// This instruction calculates a new pointer by applying an integer offset to a
/// base pointer. The result is a new pointer of the same type.
class PtrOffsetInst : public AggregateInst {
    Value basePtr;
    Value offset;

public:
    /// @brief Constructs a PtrOffsetInst object.
    ///
    /// @param basePtr The base pointer to offset from.
    /// @param offset The integer offset to apply.
    PtrOffsetInst(Value basePtr, Value offset)
        : AggregateInst(InstKind::PtrOffsetInstKind)
        , basePtr(basePtr)
        , offset(offset)
    {
        // TODO: assert(llvm::isa<glu::types::PointerTy>(*value.getType()));
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::PtrOffsetInstKind;
    }

    size_t getResultCount() const override { return 1; }
    size_t getOperandCount() const override { return 2; }

    Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return basePtr;
        case 1: return offset;
        default: llvm_unreachable("Invalid operand index");
        }
    }

    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return basePtr.getType();
    }

    Value getBasePointer() const { return basePtr; }
    Value getOffset() const { return offset; }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_PTR_OFFSET_INST_HPP
