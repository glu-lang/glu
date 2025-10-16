#ifndef GLU_GILGEN_PASSMANAGER_HPP
#define GLU_GILGEN_PASSMANAGER_HPP

#include "Basic/Diagnostic.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
#include "GIL/Module.hpp"

#include <llvm/Support/Allocator.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>

namespace glu::optimizer {

/// @brief Configuration for individual passes
struct PassConfig {
    bool enabled = true; ///< Whether the pass is enabled
    bool printBefore = false; ///< Print GIL before this pass
    bool printAfter = false; ///< Print GIL after this pass
    std::string name; ///< Pass name for identification

    PassConfig() = default;
    PassConfig(llvm::StringRef passName, bool isEnabled = true)
        : enabled(isEnabled), name(passName.str())
    {
    }
};

/// @brief Configuration for the entire pass pipeline
struct PassPipelineConfig {
    std::vector<PassConfig> passes; ///< Per-pass configurations

    /// @brief Get configuration for a specific pass by name
    PassConfig *getPassConfig(llvm::StringRef passName);

    /// @brief Enable a specific pass
    void enablePass(llvm::StringRef passName);

    /// @brief Disable a specific pass
    void disablePass(llvm::StringRef passName);

    /// @brief Enable printing before a specific pass
    void printBefore(llvm::StringRef passName);

    /// @brief Enable printing after a specific pass
    void printAfter(llvm::StringRef passName);

    /// @brief Create default configuration with all passes enabled
    static PassPipelineConfig createDefault();
};

/// @brief Efficient PassManager with direct pass execution methods
class PassManager {
private:
    PassPipelineConfig _config;
    gil::GILPrinter _printer;
    llvm::raw_ostream &_output;

    /// @brief Print the module with a description
    void printModule(gil::Module *module, llvm::StringRef description);

public:
    /// @brief Constructor
    /// @param config Configuration for the pass pipeline
    /// @param sourceManager Source manager for printing (can be nullptr)
    /// @param output Output stream for printing
    PassManager(
        PassPipelineConfig config = PassPipelineConfig::createDefault(),
        SourceManager *sourceManager = nullptr,
        llvm::raw_ostream &output = llvm::outs()
    );

    /// @brief Run all enabled passes on the module
    /// @param module The GIL module to process
    /// @param arena Memory arena for pass construction
    /// @param diagManager Diagnostic manager for error reporting
    void runPasses(
        gil::Module *module, llvm::BumpPtrAllocator &arena,
        DiagnosticManager &diagManager
    );

    /// @brief Get the current configuration
    PassPipelineConfig const &getConfig() const { return _config; }

    /// @brief Update the configuration
    void setConfig(PassPipelineConfig const &config) { _config = config; }

private:
    // Generate pass execution methods using macro
#define GIL_PASS(NAME, CLASS)                               \
    void run##CLASS(                                        \
        gil::Module *module, llvm::BumpPtrAllocator &arena, \
        DiagnosticManager &diagManager                      \
    );

#include "GILPasses.def"

#undef GIL_PASS
};

} // namespace glu::optimizer

#endif // GLU_GILGEN_PASSMANAGER_HPP
