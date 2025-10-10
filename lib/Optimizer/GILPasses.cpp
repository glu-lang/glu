#include "GILGen.hpp"
#include "GILPasses/DropLowering.hpp"
#include "GILPasses/VoidMainPass.hpp"

namespace glu::gilgen {

void GILGen::runGILPasses(gil::Module *module, llvm::BumpPtrAllocator &arena)
{
    optimizer::VoidMainPass(module, arena).visit(module);
    optimizer::DropLoweringPass(module, arena).visit(module);
}

} // namespace glu::gilgen
