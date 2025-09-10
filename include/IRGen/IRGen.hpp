#ifndef GLU_IRGEN_IRGEN_HPP
#define GLU_IRGEN_IRGEN_HPP

#include "GIL/Module.hpp"

#include <llvm/IR/Module.h>

namespace glu::irgen {

/// @brief IRGen is the main class for generating intermediate representation
/// (IR) from GIL instructions.
class IRGen {
public:
    /// @brief Generate LLVM IR from a GIL module.
    /// @param out The output LLVM module.
    /// @param mod The input GIL module.
    /// @param sourceManager The source manager for debug information, or
    /// nullptr if for no debug info.
    void generateIR(
        llvm::Module &out, glu::gil::Module *mod, SourceManager *sourceManager
    );
};

} // namespace glu::irgen

#endif // GLU_IRGEN_IRGEN_HPP
