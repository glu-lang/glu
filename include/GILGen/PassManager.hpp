#ifndef GLU_GILGEN_PASSMANAGER_HPP
#define GLU_GILGEN_PASSMANAGER_HPP

#include "Basic/Diagnostic.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
#include "GIL/Module.hpp"

#include <functional>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <vector>

namespace glu::gilgen {

/// @brief Base interface for all GIL passes
class PassBase {
public:
    virtual ~PassBase() = default;

    /// @brief Get the name of this pass
    virtual llvm::StringRef getName() const = 0;

    /// @brief Run this pass on a GIL module
    virtual void run(gil::Module *module) = 0;

    /// @brief Check if this pass requires diagnostics
    virtual bool requiresDiagnostics() const { return false; }

    /// @brief Check if this pass requires memory arena
    virtual bool requiresArena() const { return false; }
};

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

/// @brief Template wrapper for existing pass classes
template <typename PassType> class PassWrapper : public PassBase {
private:
    std::unique_ptr<PassType> _pass;
    std::string _name;
    DiagnosticManager *_diagManager;
    llvm::BumpPtrAllocator *_arena;

public:
    /// @brief Constructor for passes that don't need diagnostics or arena
    PassWrapper(llvm::StringRef name)
        : _name(name.str()), _diagManager(nullptr), _arena(nullptr)
    {
        _pass = std::make_unique<PassType>();
    }

    /// @brief Constructor for passes that need diagnostics
    PassWrapper(llvm::StringRef name, DiagnosticManager &diagManager)
        : _name(name.str()), _diagManager(&diagManager), _arena(nullptr)
    {
        _pass = std::make_unique<PassType>(diagManager);
    }

    /// @brief Constructor for passes that need arena
    PassWrapper(
        llvm::StringRef name, gil::Module *module, llvm::BumpPtrAllocator &arena
    )
        : _name(name.str()), _diagManager(nullptr), _arena(&arena)
    {
        _pass = std::make_unique<PassType>(module, arena);
    }

    /// @brief Constructor for passes that need both diagnostics and arena
    PassWrapper(
        llvm::StringRef name, DiagnosticManager &diagManager,
        gil::Module *module, llvm::BumpPtrAllocator &arena
    )
        : _name(name.str()), _diagManager(&diagManager), _arena(&arena)
    {
        _pass = std::make_unique<PassType>(diagManager, module, arena);
    }

    llvm::StringRef getName() const override { return _name; }

    void run(gil::Module *module) override { _pass->visit(module); }

    bool requiresDiagnostics() const override
    {
        return _diagManager != nullptr;
    }

    bool requiresArena() const override { return _arena != nullptr; }
};

/// @brief Factory function type for creating passes
using PassFactory = std::function<std::unique_ptr<PassBase>(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager
)>;

/// @brief Manages and executes a pipeline of GIL passes
class PassManager {
private:
    PassPipelineConfig _config;
    std::vector<std::pair<std::string, PassFactory>> _passFactories;
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

    /// @brief Register a pass factory
    /// @param name Name of the pass
    /// @param factory Factory function to create the pass
    void registerPass(llvm::StringRef name, PassFactory factory);

    /// @brief Register all default GIL passes
    void registerDefaultPasses();

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

    /// @brief Get list of registered pass names
    std::vector<std::string> getAvailablePasses() const;
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_PASSMANAGER_HPP
