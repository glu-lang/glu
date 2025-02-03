#ifndef GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
#define GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP

#include "InstBase.hpp"

namespace glu::gil {

///
///  @class AllocaInst
///  @brief Represents an allocation instruction.
///
///  This class is derived from InstBase and represents an allocation instruction
///  in the GLU GIL (Generic Intermediate Language).
///
class AllocaInst : public InstBase {
    Type _type; ///< The type of the allocation.
public:
    ///
    /// @brief Constructs an AllocaInst object.
    ///
    /// @param type The type of the allocation.
    ///
    AllocaInst(Type type) : InstBase(InstKind::AllocaInstKind), _type(type) { }

    ///
    /// @brief Gets the type of the allocation.
    ///
    /// @return The type of the allocation.
    ///
    Type getType() const { return _type; }

    virtual size_t getResultCount() const override { return 1; }

    virtual size_t getOperandCount() const override { return 1; }

    virtual Operand getOperand(size_t index) const override
    {
        assert(index == 0 && "Operand index out of range");
        return Operand(_type);
    }

    virtual Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return _type;
    }

    bool classof(const InstBase *inst)
    {
        return inst->getKind() == InstKind::AllocaInstKind;
    }

};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
