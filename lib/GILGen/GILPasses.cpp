#include "GILGen.hpp"
#include "Optimizer/PassManager.hpp"
#include "Optimizer/PassManagerOptions.hpp"

namespace glu::gilgen {

void GILGen::runGILPasses(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager, SourceManager *sourceManager,
    llvm::raw_ostream &output
)
{
    // Use PassManager from Optimizer with configuration from command line
    // options
    optimizer::PassPipelineConfig config
        = optimizer::PassManagerOptions::createConfigFromOptions();
    optimizer::PassManager passManager(config, sourceManager, output);
    passManager.runPasses(module, arena, diagManager);
}

}
