#ifndef GLU_IRGEN_IRGEN_HPP
#define GLU_IRGEN_IRGEN_HPP

#include "GIL/Module.hpp"

#include <llvm/IR/Module.h>

namespace glu::irgen {

/// @brief IRGen is the main class for generating intermediate representation
/// (IR) from GIL instructions.
class IRGen {
public:
    void generateIR(llvm::Module &out, glu::gil::Module *mod);
};

} // namespace glu::irgen

#endif // GLU_IRGEN_IRGEN_HPP
