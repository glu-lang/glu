#include "PassManager.hpp"
#include "GILPasses/DeadCodeElimination.hpp"
#include "GILPasses/DropLowering.hpp"
#include "GILPasses/UnreachableInstChecker.hpp"
#include "GILPasses/VoidMainPass.hpp"

#include <llvm/Support/WithColor.h>

namespace glu::gilgen {

// PassPipelineConfig implementation
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

    config.passes = { PassConfig("void-main", true), PassConfig("dce", true),
                      PassConfig("unreachable-checker", true),
                      PassConfig("drop-lowering", true) };

    return config;
}

// PassManager implementation
PassManager::PassManager(
    PassPipelineConfig config, SourceManager *sourceManager,
    llvm::raw_ostream &output
)
    : _config(std::move(config))
    , _printer(sourceManager, output)
    , _output(output)
{
}

void PassManager::registerPass(llvm::StringRef name, PassFactory factory)
{
    _passFactories.emplace_back(name.str(), std::move(factory));

    // Ensure we have a config entry for this pass
    if (!_config.getPassConfig(name)) {
        _config.passes.emplace_back(name, true);
    }
}

void PassManager::registerDefaultPasses()
{
    // Register VoidMainPass
    registerPass(
        "void-main",
        [](gil::Module *module, llvm::BumpPtrAllocator &arena,
           DiagnosticManager &) -> std::unique_ptr<PassBase> {
            return std::make_unique<PassWrapper<VoidMainPass>>(
                "void-main", module, arena
            );
        }
    );

    // Register DeadCodeEliminationPass
    registerPass(
        "dce",
        [](gil::Module *, llvm::BumpPtrAllocator &,
           DiagnosticManager &diagManager) -> std::unique_ptr<PassBase> {
            return std::make_unique<PassWrapper<DeadCodeEliminationPass>>(
                "dce", diagManager
            );
        }
    );

    // Register UnreachableInstChecker
    registerPass(
        "unreachable-checker",
        [](gil::Module *, llvm::BumpPtrAllocator &,
           DiagnosticManager &diagManager) -> std::unique_ptr<PassBase> {
            return std::make_unique<PassWrapper<UnreachableInstChecker>>(
                "unreachable-checker", diagManager
            );
        }
    );

    // Register DropLoweringPass
    registerPass(
        "drop-lowering",
        [](gil::Module *module, llvm::BumpPtrAllocator &arena,
           DiagnosticManager &) -> std::unique_ptr<PassBase> {
            return std::make_unique<PassWrapper<DropLoweringPass>>(
                "drop-lowering", module, arena
            );
        }
    );
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
    // Run passes in the order they appear in the configuration
    for (auto const &passConfig : _config.passes) {
        if (!passConfig.enabled) {
            continue;
        }

        // Find the factory for this pass
        auto factoryIt = std::find_if(
            _passFactories.begin(), _passFactories.end(),
            [&](auto const &pair) { return pair.first == passConfig.name; }
        );

        if (factoryIt == _passFactories.end()) {
            llvm::WithColor::warning(_output)
                << "Pass '" << passConfig.name << "' not found, skipping\n";
            continue;
        }

        // Create the pass
        auto pass = factoryIt->second(module, arena, diagManager);
        if (!pass) {
            llvm::WithColor::error(_output)
                << "Failed to create pass '" << passConfig.name << "'\n";
            continue;
        }

        // Print before if requested
        if (passConfig.printBefore) {
            printModule(module, "GIL before " + passConfig.name + " pass");
        }

        // Run the pass
        pass->run(module);

        // Print after if requested
        if (passConfig.printAfter) {
            printModule(module, "GIL after " + passConfig.name + " pass");
        }

        // Check for errors after pass and stop pipeline if there are any
        if (diagManager.hasErrors()) {
            break;
        }
    }
}

std::vector<std::string> PassManager::getAvailablePasses() const
{
    std::vector<std::string> names;
    names.reserve(_passFactories.size());

    for (auto const &pair : _passFactories) {
        names.push_back(pair.first);
    }

    return names;
}

} // namespace glu::gilgen
