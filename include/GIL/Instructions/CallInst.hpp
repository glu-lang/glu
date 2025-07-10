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
    types::FunctionTy *_functionType;
    gil::Type _returnType;

    // Method required by llvm::TrailingObjects to determine the number of
    // trailing objects
    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<Value>) const
    {
        return _argCount;
    }

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
            return getTrailingObjects<Value>()[index - 1];
        }
    }

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

    // Get the arguments
    llvm::ArrayRef<Value> getArgs() const
    {
        return { getTrailingObjects<Value>(), _argCount };
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
