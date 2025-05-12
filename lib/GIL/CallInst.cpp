#include "Instructions/CallInst.hpp"
#include "Function.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Allocator.h>

namespace glu::gil {

CallInst::CallInst(
    std::variant<Value, Function *> function, llvm::ArrayRef<Value> arguments
)
    : InstBase(InstKind::CallInstKind)
    , _function(function)
    , _argCount(arguments.size())
{
    // Copy arguments to trailing objects
    std::uninitialized_copy(
        arguments.begin(), arguments.end(), getTrailingObjects<Value>()
    );

    // Initialize function type
    if (std::holds_alternative<Function *>(_function)) {
        _functionType = std::get<Function *>(_function)->getType();
    } else {
        // TODO: _functionType =
        // llvm::cast<glu::types::FunctionTy>(std::get<Value>(_function).getType());
        assert(false && "TODO: Get function type from Value");
    }
}

CallInst *CallInst::create(
    llvm::BumpPtrAllocator &allocator, Value functionPtr,
    llvm::ArrayRef<Value> arguments
)
{
    void *mem = allocator.Allocate(
        totalSizeToAlloc<Value>(arguments.size()), alignof(CallInst)
    );
    return new (mem) CallInst(functionPtr, arguments);
}

CallInst *CallInst::create(
    llvm::BumpPtrAllocator &allocator, Function *symbol,
    llvm::ArrayRef<Value> arguments
)
{
    void *mem = allocator.Allocate(
        totalSizeToAlloc<Value>(arguments.size()), alignof(CallInst)
    );
    return new (mem) CallInst(symbol, arguments);
}

}
