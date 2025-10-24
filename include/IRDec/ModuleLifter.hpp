#ifndef GLU_IRDEC_MODULE_LIFTER_HPP
#define GLU_IRDEC_MODULE_LIFTER_HPP

#include "AST/Decls.hpp"
#include "GIL/Module.hpp"
#include <llvm/IR/Module.h>

namespace glu::irdec {

glu::ast::ModuleDecl *
liftModule(glu::ast::ASTContext &astContext, llvm::Module *llvmModule);

} // namespace glu::irdec

#endif // GLU_IRDEC_MODULE_LIFTER_HPP
