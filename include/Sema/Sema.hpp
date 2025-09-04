#ifndef GLU_SEMA_CSWALKER_HPP
#define GLU_SEMA_CSWALKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    llvm::ArrayRef<std::string> importPaths = {}
);

};

#endif // GLU_SEMA_CSWALKER_HPP
