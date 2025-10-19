#ifndef GLU_IRDEC_MODULE_LIFTER_HPP
#define GLU_IRDEC_MODULE_LIFTER_HPP

#include "GIL/Module.hpp"
#include <llvm/IR/Module.h>

namespace glu::irdec {

glu::gil::Module *liftModule(
    glu::ast::ASTContext &astContext, llvm::BumpPtrAllocator &arena,
    llvm::Module *llvmModule
);

} // namespace glu::irdec

#endif // GLU_IRDEC_MODULE_LIFTER_HPP
