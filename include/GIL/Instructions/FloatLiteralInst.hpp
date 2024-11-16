#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP

#include "InstBase.hpp"

// Forward declarations
namespace glu::gil {
class FloatLiteralInst : public ConstantInst {
protected:
    Type type;
    llvm::APFloat value;

public:
    FloatLiteralInst(Type type, llvm::APFloat value)
        : ConstantInst(InstKind::FloatLiteralInstKind), type(type), value(value)
    {
    }

    void setType(Type type) { this->type = type; }
    Type getType() const { return type; }

    void setValue(llvm::APFloat value) { this->value = value; }
    llvm::APFloat getValue() const { return value; }

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
        return inst->getKind() == InstKind::FloatLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP
