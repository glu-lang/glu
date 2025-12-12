#ifndef GLU_OPTIMIZER_PASSMANAGER_HPP
#define GLU_OPTIMIZER_PASSMANAGER_HPP

#include "Basic/Diagnostic.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/Module.hpp"
#include "Optimizer/PassManagerOptions.hpp"

#include <llvm/Support/Allocator.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>

namespace glu::optimizer {

/// @brief Efficient PassManager with direct pass execution methods
class PassManager {
private:
    DiagnosticManager &_diagManager;
    SourceManager &_sourceManager;

    gil::Module *_module = nullptr;

    /// @brief Print the module with a description
    void printModule(gil::Module *module, llvm::StringRef description);

public:
    /// @brief Constructor
    /// @param diagManager Diagnostic manager for error reporting
    /// @param sourceManager Source manager for printing (can be nullptr)
    /// @param output Output stream for printing
    /// @param module GILModule
    PassManager(
        DiagnosticManager &diagManager, SourceManager &sourceManager,
        gil::Module *module
    );

    /// @brief Run all enabled passes on the module
    void runPasses();

private:
    // Generate pass execution methods using macro
#define GIL_PASS(NAME, CLASS) void run##CLASS();

#include "GILPasses.def"

#undef GIL_PASS
};

} // namespace glu::optimizer

#endif // GLU_OPTIMIZER_PASSMANAGER_HPP
