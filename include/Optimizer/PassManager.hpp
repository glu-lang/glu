#ifndef GLU_OPTIMIZER_PASSMANAGER_HPP
#define GLU_OPTIMIZER_PASSMANAGER_HPP

#include "Basic/Diagnostic.hpp"
#include "Basic/SourceManager.hpp"
#include "GIL/GILPrinter.hpp"
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

#endif // GLU_OPTIMIZER_PASSMANAGER_HPP
