#ifndef GLU_GIL_INSTRUCTIONS_LOAD_INST_HPP
#define GLU_GIL_INSTRUCTIONS_LOAD_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

enum class LoadOwnershipKind {
    /// No ownership semantics, no OSSA
    None,
    /// The loaded value is an owned copy
    Copy,
    /// The loaded value is an owned move, ownership is transferred
    Take,
    /// The loaded value is trivial, no ownership needed
    Trivial,
};

/// @class LoadInst
/// @brief A class representing a literal instruction in the GIL.
///
/// These instructions are used to control the flow of execution in a function.
/// They have no results and are always the last instruction in a basic block.
class LoadInst : public InstBase {
    GLU_GIL_GEN_OPERAND(Value, Value, _value)
    Type _type;
    LoadOwnershipKind _ownershipKind;

public:
    LoadInst(Value value, Type type, LoadOwnershipKind ownershipKind)
        : InstBase(InstKind::LoadInstKind)
        , _value(value)
        , _type(type)
        , _ownershipKind(ownershipKind)
    {
        assert(
            llvm::isa<glu::types::PointerTy>(_value.getType().getType())
            && "LoadInst value must be a pointer type"
        );
        assert(
            llvm::cast<glu::types::PointerTy>(_value.getType().getType())
                    ->getPointee()
                == _type.getType()
            && "LoadInst value's pointee type must match the result type"
        );
    }

    LoadOwnershipKind getOwnershipKind() const { return _ownershipKind; }
    void setOwnershipKind(LoadOwnershipKind kind) { _ownershipKind = kind; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::LoadInstKind;
    }

    size_t getResultCount() const override { return 1; }

    size_t getOperandCount() const override { return 1; }

    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        assert(index == 0 && "Invalid operand index");
        return _value;
    }

    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _type;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_LOAD_INST_HPP
