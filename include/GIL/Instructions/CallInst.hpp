#ifndef GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CALL_INST_HPP

#include "InstBase.hpp"
#include "Types/FunctionTy.hpp"

#include <variant>

namespace glu::gil {
class CallInst : public InstBase {
    std::variant<Value, Function *> _function;
    llvm::SmallVector<Value, 4> _arguments;
    glu::types::FunctionTy *_functionType;

public:
    CallInst(Value functionPtr, llvm::ArrayRef<Value> arguments);
    CallInst(Function *symbol, llvm::ArrayRef<Value> arguments);

    size_t getOperandCount() const override { return _arguments.size() + 1; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        if (index == 0) {
            if (std::holds_alternative<Value>(_function)) {
                return std::get<Value>(_function);
            } else {
                return std::get<Function *>(_function);
            }
        } else {
            return _arguments[index - 1];
        }
    }

    size_t getResultCount() const override { return 1; }

    // TODO: Implement getResultType
    // Type getResultType(size_t index) const override
    // {
    //     return _functionType->getReturnType();
    // }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CallInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
