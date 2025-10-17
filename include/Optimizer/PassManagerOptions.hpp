#ifndef GLU_OPTIMIZER_PASSMANAGEROPTIONS_HPP
#define GLU_OPTIMIZER_PASSMANAGEROPTIONS_HPP

#include "PassManager.hpp"
#include <llvm/Support/CommandLine.h>

namespace glu::optimizer::options {

bool isDisabled(llvm::StringRef passName);

bool hasPrintBefore(llvm::StringRef passName);

bool hasPrintAfter(llvm::StringRef passName);

bool hasPrintBeforeEachPasses();

bool hasPrintAfterEachPasses();

} // namespace glu::optimizer::options

#endif // GLU_OPTIMIZER_PASSMANAGEROPTIONS_HPP
