#include "GILGen.hpp"
#include "GILPasses/DeadCodeElimination.hpp"
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
    DeadCodeEliminationPass(diagManager).visit(module);
    UnreachableInstChecker(diagManager).visit(module);
    DropLoweringPass(module, arena).visit(module);
}

}
