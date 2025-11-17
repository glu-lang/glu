#ifndef GLU_GILGEN_GILGEN_HPP
#define GLU_GILGEN_GILGEN_HPP

#include "Decls.hpp"
#include "GIL/Function.hpp"
#include "GIL/Module.hpp"

namespace glu {
class DiagnosticManager;
class SourceManager;
}

namespace glu::gilgen {

/// @brief The context around the GIL module being generated.
struct GlobalContext {
    llvm::DenseSet<ast::FunctionDecl *> _inlinableFunctions;

    GlobalContext([[maybe_unused]] gil::Module *module) { }
};

/// @brief Get or create a global variable in the GIL module
gil::Global *getOrCreateGlobal(gil::Module *module, ast::VarLetDecl *decl);

/// @brief Generate GIL code for a global variable
gil::Global *generateGlobal(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
);

/// @brief Generate GIL code for a global variable initializer function
gil::Function *generateGlobalInitializerFunction(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
);

/// @brief Generate GIL code for a function
gil::Function *generateFunction(
    gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx
);

/// @brief Generate a GIL module from an AST module declaration
/// @param moduleDecl The AST module declaration
/// @return A unique_ptr to the newly created GIL module
std::unique_ptr<gil::Module> generateModule(ast::ModuleDecl *moduleDecl);

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGEN_HPP
