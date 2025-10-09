#include "GIL/Module.hpp"
#include "GILGen/GILGen.hpp"
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

    glu::gil::Module *detectFuncPrototypes(llvm::Module *llvmModule);
};

}
