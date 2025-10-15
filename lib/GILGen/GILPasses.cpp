#include "GILGen.hpp"
#include "GILPasses/DropLowering.hpp"
#include "GILPasses/UnreachableInstChecker.hpp"
#include "GILPasses/VoidMainPass.hpp"

namespace glu::gilgen {

void GILGen::runGILPasses(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager
)
{
    VoidMainPass(module, arena).visit(module);
    DropLoweringPass(module, arena).visit(module);
    // After DCE (if implemented), check for reachable unreachable instructions
    // For now, we run this after other passes
    UnreachableInstChecker(diagManager).visit(module);
}

}
