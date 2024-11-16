#ifndef GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP

#include "InstBase.hpp"

// Forward declarations
namespace glu::gil {

class IntegerLiteralInst : public ConstantInst {
protected:
    Type type;
    llvm::APInt value;

public:
    IntegerLiteralInst(Type type, llvm::APInt value)
        : ConstantInst(InstKind::IntegerLiteralInstKind)
        , type(type)
        , value(value)
    {
    }

    void setType(Type newType) { this->type = newType; }
    Type getType() const { return type; }

    void setValue(llvm::APInt newValue) { this->value = newValue; }
    llvm::APInt getValue() const { return value; }

    Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return getType();
        case 1: return getValue();
        default: llvm_unreachable("Invalid operand index");
        }
    }

    Type getResultType(size_t index) const override { return type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntegerLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
