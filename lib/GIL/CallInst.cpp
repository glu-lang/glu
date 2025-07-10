#include "Instructions/CallInst.hpp"
#include "Function.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Allocator.h>

namespace glu::gil {

CallInst::CallInst(
    Type returnType, std::variant<Value, Function *> function,
    llvm::ArrayRef<Value> arguments
)
    : InstBase(InstKind::CallInstKind)
    , _function(function)
    , _argCount(arguments.size())
    , _returnType(returnType)
{
    // Copy arguments to trailing objects
    std::uninitialized_copy(
        arguments.begin(), arguments.end(), getTrailingObjects<Value>()
    );

    // Initialize function type
    if (std::holds_alternative<Function *>(_function)) {
        _functionType = std::get<Function *>(_function)->getType();
    } else {
        _functionType = llvm::cast<glu::types::FunctionTy>(
            std::get<Value>(_function).getType().getType()
        );
    }
    assert(
        _functionType->getReturnType() == returnType.getType()
        && "Function type does not match return type"
    );
}

CallInst *CallInst::create(
    llvm::BumpPtrAllocator &allocator, Type returnType, Value functionPtr,
    llvm::ArrayRef<Value> arguments
)
{
    void *mem = allocator.Allocate(
        totalSizeToAlloc<Value>(arguments.size()), alignof(CallInst)
    );
    return new (mem) CallInst(returnType, functionPtr, arguments);
}

CallInst *CallInst::create(
    llvm::BumpPtrAllocator &allocator, Type returnType, Function *symbol,
    llvm::ArrayRef<Value> arguments
)
{
    void *mem = allocator.Allocate(
        totalSizeToAlloc<Value>(arguments.size()), alignof(CallInst)
    );
    return new (mem) CallInst(returnType, symbol, arguments);
}

}
