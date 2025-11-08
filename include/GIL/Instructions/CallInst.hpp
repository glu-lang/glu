#ifndef GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CALL_INST_HPP

#include "InstBase.hpp"
#include "Types/FunctionTy.hpp"

#include <llvm/Support/TrailingObjects.h>
#include <variant>

namespace glu::gil {
class CallInst final : public InstBase,
                       private llvm::TrailingObjects<CallInst, Value> {
    GLU_GIL_GEN_OPERAND_LIST_TRAILING_OBJECTS(CallInst, _argCount, Value, Args)

    std::variant<Value, Function *> _function;
    types::FunctionTy *_functionType;
    gil::Type _returnType;

    // Private constructor
    CallInst(
        Type returnType, std::variant<Value, Function *> function,
        llvm::ArrayRef<Value> arguments
    );

public:
    // CallInst instance creators
    static CallInst *create(
        llvm::BumpPtrAllocator &allocator, Type returnType, Value functionPtr,
        llvm::ArrayRef<Value> arguments
    );
    static CallInst *create(
        llvm::BumpPtrAllocator &allocator, Type returnType, Function *symbol,
        llvm::ArrayRef<Value> arguments
    );

    size_t getOperandCount() const override { return _argCount + 1; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        if (index == 0) {
            if (std::holds_alternative<Value>(_function)) {
                return std::get<Value>(_function);
            } else {
                return std::get<Function *>(_function);
            }
        } else {
            return getArgs()[index - 1];
        }
    }

    std::variant<Value, Function *> getFunction() const { return _function; }

    Function *getFunctionOrNull() const
    {
        if (std::holds_alternative<Value>(_function)) {
            return nullptr;
        } else {
            return std::get<Function *>(_function);
        }
    }

    std::optional<Value> getFunctionPtrValue() const
    {
        if (std::holds_alternative<Value>(_function)) {
            return std::get<Value>(_function);
        } else {
            return std::nullopt;
        }
    }

    size_t getResultCount() const override
    {
        return !llvm::isa<types::VoidTy>(_functionType->getReturnType());
    }

    Type getResultType(size_t index) const override
    {
        assert(index < getResultCount() && "Index out of bounds");
        return _returnType;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CallInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
