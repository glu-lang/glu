#ifndef GLU_GIL_INSTRUCTIONS_STORE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STORE_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

enum class StoreOwnershipKind {
    /// No known ownership semantics, no OSSA
    None,
    /// The stored value is being initialized (uninitialized -> initialized)
    Init,
    /// The stored value is being assigned (the previous contents will be
    /// dropped)
    Set,
    /// The stored value is trivial, no ownership needed
    Trivial,
};

/// @class StoreInst
/// @brief A class representing a literal instruction in the GIL.
///
/// These instructions are used to control the flow of execution in a function.
/// They have no results and are always the last instruction in a basic block.
class StoreInst : public InstBase {
    GLU_GIL_GEN_OPERAND(Source, Value, _source)
    GLU_GIL_GEN_OPERAND(Dest, Value, _dest)
    StoreOwnershipKind _ownershipKind;

public:
    StoreInst(
        Value source, Value dest,
        StoreOwnershipKind ownershipKind = StoreOwnershipKind::None
    )
        : InstBase(InstKind::StoreInstKind)
        , _source(source)
        , _dest(dest)
        , _ownershipKind(ownershipKind)
    {
    }

    StoreOwnershipKind getOwnershipKind() const { return _ownershipKind; }
    void setOwnershipKind(StoreOwnershipKind kind) { _ownershipKind = kind; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StoreInstKind;
    }

    Type getResultType([[maybe_unused]] size_t index) const override
    {
        llvm_unreachable("StoreInst has no result type");
    }
    size_t getResultCount() const override { return 0; }
    size_t getOperandCount() const override { return 2; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        if (index == 0)
            return _source;
        if (index == 1)
            return _dest;
        llvm_unreachable("Invalid operand index");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STORE_INST_HPP
