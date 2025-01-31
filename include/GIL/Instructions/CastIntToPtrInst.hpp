#ifndef GLU_GIL_CAST_INT_TO_PTR_INST_HPP
#define GLU_GIL_CAST_INT_TO_PTR_INST_HPP

#include "InstBase.hpp"

class CastIntToPtrInst : public InstBase {
    Value _value;
    Type _type;

public:
    CastIntToPtrInst(Value value, Type type)
        : InstBase(InstKind::CastPtrToIntInstKind), _value(value), _type(type)
    {
    }

    size_t getOperandCount() const override { return 1; }
    Operand getOperand(size_t index) const override
    {
        if (index == 0) {
            return _value;
        } else {
            return {};
        }
    }

    size_t getResultCount() const override { return 1; }
    Type getResultType(size_t index) const override { return _type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CastIntToPtrInstKind;
    }

};

#endif // GLU_GIL_CAST_INT_TO_PTR_INST_HPP
