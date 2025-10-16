#include "PassManager.hpp"

#include <llvm/Support/WithColor.h>

namespace glu::gilgen {

PassManager::PassManager(
    PassPipelineConfig config, SourceManager *sourceManager,
    llvm::raw_ostream &output
)
    : _config(std::move(config))
    , _printer(sourceManager, output)
    , _output(output)
{
}

void PassManager::printModule(gil::Module *module, llvm::StringRef description)
{
    llvm::WithColor(llvm::outs(), llvm::raw_ostream::CYAN, true)
        << "// " << description << "\n";
    _output << "\n";
    _printer.visit(module);
    _output << "\n";
    llvm::WithColor(llvm::outs(), llvm::raw_ostream::CYAN, true)
        << "// End " << description << "\n\n";
}

void PassManager::runPasses(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager
)
{
    for (auto const &passConfig : _config.passes) {
        if (!passConfig.enabled) {
            continue;
        }

        if (passConfig.printBefore) {
            printModule(module, "GIL before " + passConfig.name + " pass");
        }

        bool passFound = false;
#define GIL_PASS(NAME, CLASS)                   \
    if (passConfig.name == NAME) {              \
        run##CLASS(module, arena, diagManager); \
        passFound = true;                       \
    }

#include "GILPasses.def"

#undef GIL_PASS

        if (!passFound) {
            llvm::WithColor::warning(_output)
                << "Pass '" << passConfig.name << "' not found, skipping\n";
            continue;
        }

        if (passConfig.printAfter) {
            printModule(module, "GIL after " + passConfig.name + " pass");
        }

        if (diagManager.hasErrors()) {
            break;
        }
    }
}

PassConfig *PassPipelineConfig::getPassConfig(llvm::StringRef passName)
{
    for (auto &config : passes) {
        if (config.name == passName) {
            return &config;
        }
    }
    return nullptr;
}

void PassPipelineConfig::enablePass(llvm::StringRef passName)
{
    if (auto *config = getPassConfig(passName)) {
        config->enabled = true;
    }
}

void PassPipelineConfig::disablePass(llvm::StringRef passName)
{
    if (auto *config = getPassConfig(passName)) {
        config->enabled = false;
    }
}

void PassPipelineConfig::printBefore(llvm::StringRef passName)
{
    if (auto *config = getPassConfig(passName)) {
        config->printBefore = true;
    }
}

void PassPipelineConfig::printAfter(llvm::StringRef passName)
{
    if (auto *config = getPassConfig(passName)) {
        config->printAfter = true;
    }
}

PassPipelineConfig PassPipelineConfig::createDefault()
{
    PassPipelineConfig config;

#define GIL_PASS(NAME, CLASS) config.passes.emplace_back(NAME, true);

#include "GILPasses.def"

#undef GIL_PASS

    return config;
}

} // namespace glu::gilgen
