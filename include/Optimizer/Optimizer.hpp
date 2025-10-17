#ifndef GLU_OPTIMIZER_OPTIMIZER_HPP
#define GLU_OPTIMIZER_OPTIMIZER_HPP

#include <llvm/Support/Allocator.h>
#include <llvm/Support/raw_ostream.h>

namespace glu {

class DiagnosticManager;
class SourceManager;

namespace gil {
class Module;
}

class Optimizer {
private:
    DiagnosticManager &_diagManager;
    SourceManager *_sourceManager;
    llvm::raw_ostream &_output;

public:
    /// @brief Constructor
    /// @param diagManager Diagnostic manager for error reporting
    /// @param sourceManager Source manager for printing (optional)
    /// @param output Output stream for printing
    Optimizer(
        DiagnosticManager &diagManager, SourceManager *sourceManager = nullptr,
        llvm::raw_ostream &output = llvm::outs()
    );

    ~Optimizer() = default;

    /// @brief Run GIL passes using the PassManager
    /// Automatically configures passes from command line options
    /// @param module The GIL module to process
    /// @param arena Memory arena of GIL functions
    void runGILPasses(gil::Module *module, llvm::BumpPtrAllocator &arena);
};

} // namespace glu

#endif // GLU_OPTIMIZER_OPTIMIZER_HPP

