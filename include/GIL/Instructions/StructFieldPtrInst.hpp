#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

class StructFieldPtrInst : public InstBase {
    Value _structValue;
    Member _member;
    Type _ptr;

public:
    StructFieldPtrInst(Value structValue, Member member)
        : InstBase(InstKind::StructFieldPtrInstKind)
        , _structValue(structValue)
        , _member(member)
    {
    }

    Value getStructValue() const { return _structValue; }

    Member getMember() const { return _member; }

    virtual size_t getResultCount() const override { return 1; }

    virtual size_t getOperandCount() const override { return 2; }

    virtual Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return _structValue;
        case 1: return _member;
        default: llvm_unreachable("Invalid operand index");
        }
    }

    // TODO: Implement Member class to create PointerTy from member type
    // virtual Type getResultType(size_t index) const override
    // {
    //     assert(index == 0 && "Result index out of range");
    //     return glu::types::PointerTy member.getType();
    // }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructFieldPtrInstKind;
    }
};

} // namespace glu::gil

#endif GLU_GIL_INSTRUCTIONS_INST_BASE_HPP
