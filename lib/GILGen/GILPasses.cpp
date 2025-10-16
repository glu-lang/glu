#include "GILGen.hpp"
#include "GILPasses/DeadCodeElimination.hpp"
#include "GILPasses/DropLowering.hpp"
#include "GILPasses/UnreachableInstChecker.hpp"
#include "GILPasses/VoidMainPass.hpp"
#include "PassManager.hpp"
#include "PassManagerOptions.hpp"

namespace glu::gilgen {

void GILGen::runGILPasses(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager, SourceManager *sourceManager,
    llvm::raw_ostream &output
)
{
    // Use PassManager with configuration from command line options
    PassPipelineConfig config = PassManagerOptions::createConfigFromOptions();
    PassManager passManager(config, sourceManager, output);
    passManager.registerDefaultPasses();
    passManager.runPasses(module, arena, diagManager);
}

}
