#ifndef GLU_GIL_INSTRUCTIONS_DEBUG_INST_HPP
#define GLU_GIL_INSTRUCTIONS_DEBUG_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

class DebugInst : public InstBase {
public:
    enum class DebugBindingType {
        Let,
        var,
        Arg,
    };

private:
    llvm::StringRef _name;
    Value _value;
    DebugBindingType _bindingType;

public:
    DebugInst(
        llvm::StringRef name, Value value, DebugBindingType bindingType,
        SourceLocation loc
    )
        : InstBase(InstKind::DebugInstKind)
        , _value(value)
        , _name(name)
        , _bindingType(bindingType)
    {
        this->setLocation(loc);
    }

    llvm::StringRef getName() const { return _name; }
    Value getValue() const { return _value; }
    DebugBindingType getBindingType() const { return _bindingType; }
    SourceLocation getLocation() const { return InstBase::getLocation(); }

    size_t getResultCount() const override { return 0; }
    Type getResultType([[maybe_unused]] size_t index) const override
    {
        assert(false && "DebugInst has no results");
        return Type();
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
