#ifndef GLU_IRDEC_MODULE_LIFTER_HPP
#define GLU_IRDEC_MODULE_LIFTER_HPP

#include "AST/Decls.hpp"

#include <llvm/IR/Module.h>

namespace glu::irdec {

struct ModuleLiftingContext {
    glu::ast::ASTContext &ast;
    std::vector<glu::ast::DeclBase *> rootDecls;

    // TypeLifter and DITypeLifter caches
    llvm::DenseMap<llvm::DIType const *, glu::ast::DeclBase *> diTypeCache;
    llvm::DenseMap<llvm::Type const *, glu::ast::DeclBase *> typeCache;

    ModuleLiftingContext(glu::ast::ASTContext &astContext) : ast(astContext) { }

    glu::ast::DeclBase *
    addToNamespace(llvm::DIScope const *parent, glu::ast::DeclBase *content);
};

glu::types::TypeBase *lift(llvm::Type *type, ModuleLiftingContext &context);
glu::types::TypeBase *
lift(llvm::DIType const *diType, ModuleLiftingContext &context);

/// @brief Lift an LLVM module to a GLU module declaration. This is the main
/// entry point for module lifting.
/// @param astContext The AST context to use for lifting.
/// @param llvmModule The LLVM module to lift.
/// @return The lifted GLU module declaration.
glu::ast::ModuleDecl *
liftModule(glu::ast::ASTContext &astContext, llvm::Module *llvmModule);

} // namespace glu::irdec

#endif // GLU_IRDEC_MODULE_LIFTER_HPP
