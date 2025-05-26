#ifndef GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CALL_INST_HPP

#include "InstBase.hpp"
#include "Types/FunctionTy.hpp"

#include <llvm/Support/TrailingObjects.h>
#include <variant>

namespace glu::gil {
class CallInst final : public InstBase,
                       private llvm::TrailingObjects<CallInst, Value> {
    using TrailingArgs = llvm::TrailingObjects<CallInst, Value>;
    friend TrailingArgs;

    std::variant<Value, Function *> _function;
    unsigned _argCount;
    glu::types::FunctionTy *_functionType;

    // Method required by llvm::TrailingObjects to determine the number of
    // trailing objects
    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<Value>) const
    {
        return _argCount;
    }

    // Private constructor
    CallInst(
        std::variant<Value, Function *> function,
        llvm::ArrayRef<Value> arguments
    );

public:
    // CallInst instance creators
    static CallInst *create(
        llvm::BumpPtrAllocator &allocator, Value functionPtr,
        llvm::ArrayRef<Value> arguments
    );
    static CallInst *create(
        llvm::BumpPtrAllocator &allocator, Function *symbol,
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
            return getTrailingObjects<Value>()[index - 1];
        }
    }

    // Get the arguments
    llvm::ArrayRef<Value> getArgs() const
    {
        return { getTrailingObjects<Value>(), _argCount };
    }

    size_t getResultCount() const override { return 1; }

    // TODO: Implement getResultType
    Type getResultType(size_t index) const override
    {
        // return _functionType->getReturnType();
        return Type();
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CallInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
