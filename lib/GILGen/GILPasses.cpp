#include "GILGen.hpp"
#include "GILPasses/DropLowering.hpp"
#include "GILPasses/VoidMainPass.hpp"

namespace glu::gilgen {

void GILGen::runGILPasses(gil::Module *module, llvm::BumpPtrAllocator &arena)
{
    VoidMainPass(module, arena).visit(module);
    DropLoweringPass(module, arena).visit(module);
}

}
