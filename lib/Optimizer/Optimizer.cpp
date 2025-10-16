#include "Optimizer.hpp"
#include "PassManager.hpp"
#include "PassManagerOptions.hpp"

namespace glu {

Optimizer::Optimizer(
    DiagnosticManager &diagManager, SourceManager *sourceManager,
    llvm::raw_ostream &output
)
    : _diagManager(diagManager), _sourceManager(sourceManager), _output(output)
{
}

void Optimizer::runGILPasses(
    gil::Module *module, llvm::BumpPtrAllocator &GILFunctionsArena
)
{
    optimizer::PassPipelineConfig config
        = optimizer::PassManagerOptions::createConfigFromOptions();
    optimizer::PassManager passManager(config, _sourceManager, _output);
    passManager.runPasses(module, GILFunctionsArena, _diagManager);
}

} // namespace glu
