#include "Function.hpp"
#include "Instructions/CallInst.hpp"

#include <llvm/ADT/ArrayRef.h>

namespace glu::gil {
CallInst::CallInst(Value functionPtr, llvm::ArrayRef<Value> arguments)
    : InstBase(InstKind::CallInstKind)
    , _function(functionPtr)
    , _arguments(arguments)
{
    // TODO: _functionType =
    // llvm::cast<glu::types::FunctionTy>(functionPtr.getType());
    assert(false && "TODO");
}
CallInst::CallInst(Function *symbol, llvm::ArrayRef<Value> arguments)
    : InstBase(InstKind::CallInstKind), _function(symbol), _arguments(arguments)
{
    _functionType = symbol->getType();
}
}
