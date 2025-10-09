#include "GIL/Module.hpp"
#include "GILGen/GILGen.hpp"
#include "IRDec/TypeLifter.hpp"
#include "llvm/IR/Function.h"
#include <llvm/IR/Module.h>

namespace glu::irdec {

class FuncProtoDetector {
    glu::ast::ASTContext &_astContext;
    glu::gilgen::GlobalContext _globalCtx;

public:
    FuncProtoDetector(
        glu::ast::ASTContext &astContext, llvm::BumpPtrAllocator &arena
    )
        : _astContext(astContext)
        , _globalCtx(arena.Allocate<glu::gil::Module>(), arena)
    {
    }

    ~FuncProtoDetector() = default;

    glu::gil::Module *detectFuncPrototypes(llvm::Module *llvmModule)
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
    }
};

}
