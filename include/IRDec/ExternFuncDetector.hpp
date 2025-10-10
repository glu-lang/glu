#ifndef GLU_IRDEC_EXTERN_FUNC_DETECTOR_HPP
#define GLU_IRDEC_EXTERN_FUNC_DETECTOR_HPP

#include "GILGen/GILGen.hpp"
#include <llvm/IR/Module.h>

namespace glu::irdec {

glu::gil::Module *createGilModuleFromLLVMModule(
    glu::ast::ASTContext &astContext, llvm::BumpPtrAllocator &arena,
    llvm::Module *llvmModule
);

} // namespace glu::irdec

#endif // GLU_IRDEC_EXTERN_FUNC_DETECTOR_HPP
