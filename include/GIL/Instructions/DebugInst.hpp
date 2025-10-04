#ifndef GLU_GIL_INSTRUCTIONS_DEBUG_INST_HPP
#define GLU_GIL_INSTRUCTIONS_DEBUG_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

enum class DebugBindingType {
    Let,
    Var,
    Arg,
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os, DebugBindingType t)
{
    switch (t) {
    case DebugBindingType::Let: return os << "let";
    case DebugBindingType::Var: return os << "var";
    case DebugBindingType::Arg: return os << "arg";
    }
    llvm_unreachable("Invalid DebugBindingType");
}

class DebugInst : public InstBase {
    llvm::StringRef _name;
    Value _value;
    DebugBindingType _bindingType;

public:
    DebugInst(llvm::StringRef name, Value value, DebugBindingType bindingType)
        : InstBase(InstKind::DebugInstKind)
        , _name(name)
        , _value(value)
        , _bindingType(bindingType)
    {
    }

    llvm::StringRef getName() const { return _name; }
    Value getValue() const { return _value; }
    DebugBindingType getBindingType() const { return _bindingType; }

    size_t getResultCount() const override { return 0; }
    Type getResultType([[maybe_unused]] size_t index) const override
    {
        llvm_unreachable("DebugInst has no results");
    }

    size_t getOperandCount() const override { return 1; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        assert(index == 0 && "Invalid operand index");
        return Operand(_value);
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::DebugInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_DEBUG_INST_HPP
