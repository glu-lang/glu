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
    static CallInst *
    create(Type returnType, Value functionPtr, llvm::ArrayRef<Value> arguments);
    static CallInst *
    create(Type returnType, Function *symbol, llvm::ArrayRef<Value> arguments);

    // Custom delete operator for TrailingObjects
    void operator delete(void *ptr) { ::operator delete(ptr); }

    std::variant<Value, Function *> getFunction() const { return _function; }

    void setFunction(std::variant<Value, Function *> function)
    {
        if (std::holds_alternative<Value>(function)) {
            setFunction(std::get<Value>(function));
        } else {
            setFunction(std::get<Function *>(function));
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

    void setFunction(Value functionPtr)
    {
        assert(
            _functionType
                == llvm::cast<glu::types::PointerTy>(functionPtr.getType())
                       ->getPointee()
            && "Function type mismatch"
        );
        _function = functionPtr;
    }

    void setFunction(Function *function);

    size_t getResultCountImpl() const
    {
        return !llvm::isa<types::VoidTy>(_functionType->getReturnType());
    }

    Type getResultTypeImpl(size_t index) const
    {
        assert(index < getResultCountImpl() && "Index out of bounds");
        return _returnType;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CallInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CALL_INST_HPP
