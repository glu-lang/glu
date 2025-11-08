#ifndef GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP

#include "ConstantInst.hpp"

namespace glu::gil {

class IntegerLiteralInst final : public ConstantInst {
    GLU_GIL_GEN_OPERAND(Type, Type, _type)
    GLU_GIL_GEN_OPERAND(Value, llvm::APInt, _value)

public:
    IntegerLiteralInst(Type type, llvm::APInt const &value)
        : ConstantInst(InstKind::IntegerLiteralInstKind)
        , _type(type)
        , _value(value)
    {
    }

    static IntegerLiteralInst *create(Type type, llvm::APInt const &value)
    {
        return new IntegerLiteralInst(type, value);
    }

    Type getResultType() const { return _type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntegerLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
