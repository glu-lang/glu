#include "IRDec/FuncProtoDetector.hpp"
#include "IRDec/TypeLifter.hpp"
#include "llvm/IR/Function.h"

namespace glu::irdec {

glu::gil::Module *
FuncProtoDetector::detectFuncPrototypes(llvm::Module *llvmModule)
{
    TypeLifter typeLifter(_astContext);

    for (auto &func : llvmModule->functions()) {
        if (func.isDeclaration()) {
            auto type = typeLifter.lift(func.getFunctionType());
            if (auto funcType
                = llvm::dyn_cast_if_present<glu::types::FunctionTy>(type)) {
                glu::gil::Function *gilFunc = new (_globalCtx.arena)
                    glu::gil::Function(func.getName(), funcType, nullptr);
                _globalCtx.module->addFunction(gilFunc);
            }
        }
    }
    return _globalCtx.module;
};
}
