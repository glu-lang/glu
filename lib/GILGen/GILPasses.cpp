#include "GILGen.hpp"
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
    passManager.runPasses(module, arena, diagManager);
}

}
