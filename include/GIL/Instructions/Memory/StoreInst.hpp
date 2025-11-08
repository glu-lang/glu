#ifndef GLU_GIL_INSTRUCTIONS_STORE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STORE_INST_HPP

#include "../InstBase.hpp"

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
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STORE_INST_HPP
