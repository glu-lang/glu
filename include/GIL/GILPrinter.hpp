#ifndef GLU_GIL_GILPRINTER_HPP
#define GLU_GIL_GILPRINTER_HPP

#include "Basic/SourceManager.hpp"

#include "Module.hpp"

#include <llvm/Support/raw_ostream.h>

namespace glu::gil {
void printModule(Module *module, llvm::raw_ostream &out, SourceManager *sm);

void printFunction(
    Function *function, llvm::raw_ostream &out, SourceManager *sm = nullptr
);
} // namespace glu::gil

#endif // GLU_GIL_GILPRINTER_HPP
