#ifndef GLU_GIL_INSTRUCTIONS_STRING_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRING_LITERAL_INST_HPP

#include "ConstantInst.hpp"

namespace glu::gil {

class StringLiteralInst : public ConstantInst {
protected:
    GLU_GIL_GEN_OPERAND(Type, Type, _type)
    GLU_GIL_GEN_OPERAND(Value, llvm::StringRef, _value)

public:
    StringLiteralInst(Type type, llvm::StringRef value)
        : ConstantInst(InstKind::StringLiteralInstKind)
        , _type(type)
        , _value(value)
    {
    }

    Type getResultType([[maybe_unused]] size_t index) const override
    {
        return _type;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StringLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRING_LITERAL_INST_HPP
