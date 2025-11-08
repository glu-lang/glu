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
    GLU_GIL_GEN_OPERAND(BasePtr, Value, _basePtr)
    GLU_GIL_GEN_OPERAND(Offset, Value, _offset)

public:
    /// @brief Constructs a PtrOffsetInst object.
    ///
    /// @param basePtr The base pointer to offset from.
    /// @param offset The integer offset to apply.
    PtrOffsetInst(Value basePtr, Value offset)
        : AggregateInst(InstKind::PtrOffsetInstKind)
        , _basePtr(basePtr)
        , _offset(offset)
    {
        assert(llvm::isa<glu::types::PointerTy>(*basePtr.getType()));
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::PtrOffsetInstKind;
    }

    size_t getResultCount() const override { return 1; }

    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return _basePtr.getType();
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_PTR_OFFSET_INST_HPP
