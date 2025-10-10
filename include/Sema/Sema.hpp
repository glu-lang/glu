#ifndef GLU_SEMA_CSWALKER_HPP
#define GLU_SEMA_CSWALKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

class ImportManager;
class ScopeTable;

/// @brief Constrains the given main module by performing semantic analysis.
/// This includes type checking, scope resolution, and other semantic checks.
/// The function modifies the AST in place to reflect the results of the
/// semantic analysis.
/// @param module The root module declaration of the AST to be constrained.
/// @param diagManager The diagnostic manager to report errors and warnings.
/// @param importPaths Optional array of import paths to resolve imports.
/// @return Returns the constrained module declaration, or nullptr if errors
/// occurred.
void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    llvm::ArrayRef<std::string> importPaths = {}, bool dumpConstraints = false
);

/// @brief Constrains the given main module by performing full semantic analysis
/// using an external ImportManager. This allows the caller to maintain control
/// over import resolution and access imported module information afterward.
/// @param module The root module declaration of the AST to be constrained.
/// @param diagManager The diagnostic manager to report errors and warnings.
/// @param importManager The import manager to handle import declarations.
void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    ImportManager *importManager, bool dumpConstraints = false
);

/// @brief Fast version of constrainAST that does not fully check the contents
/// of function bodies. This is useful for quickly resolving imports without
/// needing to fully analyze the entire AST.
/// @param module The root module declaration of the AST to be constrained.
/// @param diagManager The diagnostic manager to report errors and warnings.
/// @param importManager The import manager to handle import declarations.
/// @return Returns the constrained module declaration, or nullptr if errors
/// occurred.
ScopeTable *fastConstrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    ImportManager *importManager
);

void runLocalCSWalker(
    ScopeTable *scope, ast::ASTNode *node, glu::DiagnosticManager &diagManager,
    glu::ast::ASTContext *context, llvm::raw_ostream *dumpConstraints
);
};

#endif // GLU_SEMA_CSWALKER_HPP
