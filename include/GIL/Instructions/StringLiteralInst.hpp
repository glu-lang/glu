#ifndef GLU_GIL_INSTRUCTIONS_STRING_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRING_LITERAL_INST_HPP

#include "ConstantInst.hpp"

namespace glu::gil {

class StringLiteralInst : public ConstantInst {
protected:
    Type type;
    std::string value;

public:
    StringLiteralInst(Type type, std::string value)
        : ConstantInst(InstKind::StringLiteralInstKind)
        , type(type)
        , value(value)
    {
    }

    void setType(Type type) { this->type = type; }
    Type getType() const { return type; }

    void setValue(std::string value) { this->value = std::move(value); }
    llvm::StringRef getValue() const { return value; }

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
        return inst->getKind() == InstKind::StringLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRING_LITERAL_INST_HPP