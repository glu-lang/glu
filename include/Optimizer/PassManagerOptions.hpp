#ifndef GLU_GILGEN_PASSMANAGEROPTIONS_HPP
#define GLU_GILGEN_PASSMANAGEROPTIONS_HPP

#include "PassManager.hpp"
#include <llvm/Support/CommandLine.h>

namespace glu::optimizer {

/// @brief Command line options for the PassManager
class PassManagerOptions {
private:
    static llvm::cl::list<std::string> DisablePasses;
    static llvm::cl::list<std::string> PrintBeforePasses;
    static llvm::cl::list<std::string> PrintAfterPasses;

public:
    /// @brief Create a PassPipelineConfig from the current command line options
    static PassPipelineConfig createConfigFromOptions()
    {
        PassPipelineConfig config = PassPipelineConfig::createDefault();

        // Disable requested passes
        for (auto const &passName : DisablePasses) {
            config.disablePass(passName);
        }

        // Configure printing before specific passes
        for (auto const &passName : PrintBeforePasses) {
            config.printBefore(passName);
        }

        // Configure printing after specific passes
        for (auto const &passName : PrintAfterPasses) {
            config.printAfter(passName);
        }

        return config;
    }
};

} // namespace glu::optimizer

#endif // GLU_GILGEN_PASSMANAGEROPTIONS_HPP
