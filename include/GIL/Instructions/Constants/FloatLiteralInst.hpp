#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP

#include "ConstantInst.hpp"

namespace glu::gil {

class FloatLiteralInst final : public ConstantInst {
    GLU_GIL_GEN_OPERAND(Type, Type, _type)
    GLU_GIL_GEN_OPERAND(Value, llvm::APFloat, _value)

public:
    FloatLiteralInst(Type type, llvm::APFloat const &value)
        : ConstantInst(InstKind::FloatLiteralInstKind)
        , _type(type)
        , _value(value)
    {
    }

    static FloatLiteralInst *create(Type type, llvm::APFloat const &value)
    {
        return new FloatLiteralInst(type, value);
    }

    Type getResultType() const { return _type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FloatLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP
