#include "ModuleLifter.hpp"
#include "GILGen/GILGen.hpp"
#include "TypeLifter.hpp"
#include "llvm/IR/Function.h"

namespace glu::irdec {

class ModuleLifter {
    glu::ast::ASTContext &_astContext;
    glu::gilgen::GlobalContext _globalCtx;
    llvm::Module *_llvmModule;

public:
    ModuleLifter(
        glu::ast::ASTContext &astContext, llvm::BumpPtrAllocator &arena,
        llvm::Module *llvmModule
    )
        : _astContext(astContext)
        , _globalCtx(new(arena) glu::gil::Module(llvmModule->getName()), arena)
        , _llvmModule(llvmModule)
    {
    }

    ~ModuleLifter() = default;

    glu::gil::Module *detectExternalFunctions()
    {
        TypeLifter typeLifter(_astContext);

        for (auto &func : _llvmModule->functions()) {
            if (!func.isDeclaration()
                && func.getLinkage() == llvm::Function::ExternalLinkage) {
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

glu::gil::Module *liftModule(
    glu::ast::ASTContext &astContext, llvm::BumpPtrAllocator &arena,
    llvm::Module *llvmModule
)
{
    ModuleLifter lifter(astContext, arena, llvmModule);
    return lifter.detectExternalFunctions();
}

} // namespace glu::irdec
