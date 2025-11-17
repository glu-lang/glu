#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "GILGen.hpp"
#include "GILGenExprs.hpp"
#include "Scope.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

struct GILGenModule : ast::ASTVisitor<GILGenModule, void> {
    gil::Module *module;
    GlobalContext &globalCtx;

    GILGenModule(gil::Module *mod, GlobalContext &gCtx)
        : module(mod), globalCtx(gCtx)
    {
    }

    void visitModuleDecl(ast::ModuleDecl *modDecl)
    {
        for (auto *decl : modDecl->getDecls()) {
            visit(decl);
        }
    }

    void visitNamespaceDecl(ast::NamespaceDecl *nsDecl)
    {
        for (auto *decl : nsDecl->getDecls()) {
            visit(decl);
        }
    }

    void visitFunctionDecl(ast::FunctionDecl *fnDecl)
    {
        if (fnDecl->getBody() == nullptr) {
            // If the function has no body, we skip it
            return;
        }
        generateFunction(module, fnDecl, globalCtx);
    }

    void visitVarLetDecl(ast::VarLetDecl *varDecl)
    {
        // Global variable or constant
        generateGlobal(module, varDecl, globalCtx);
    }
};

gil::Global *getOrCreateGlobal(gil::Module *module, ast::VarLetDecl *decl)
{
    for (auto &g : module->getGlobals()) {
        if (g.getDecl() == decl) {
            return &g;
        }
    }

    auto *global = new gil::Global(
        decl->getName(), decl->getType(), decl->getValue() != nullptr, decl
    );
    module->addGlobal(global);
    return global;
}

gil::Global *generateGlobal(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
)
{
    auto *global = getOrCreateGlobal(module, decl);
    if (decl->getValue() != nullptr) {
        global->setInitializer(
            generateGlobalInitializerFunction(module, decl, globalCtx)
        );
    }
    return global;
}

std::unique_ptr<gil::Module> generateModule(ast::ModuleDecl *moduleDecl)
{
    auto gilModule = std::make_unique<gil::Module>(moduleDecl);
    GlobalContext globalCtx(gilModule.get());

    // Generate GIL for all declarations in the module
    GILGenModule(gilModule.get(), globalCtx).visitModuleDecl(moduleDecl);

    // Generate GIL for all inlinable functions from other modules
    while (!globalCtx._inlinableFunctions.empty()) {
        auto fn = *globalCtx._inlinableFunctions.begin();
        globalCtx._inlinableFunctions.erase(fn);
        if (auto *gilFn = gilModule->getFunctionByDecl(fn);
            gilFn && gilFn->getBasicBlockCount()) {
            // Already generated
            continue;
        }
        generateFunction(gilModule.get(), fn, globalCtx);
    }

    return gilModule;
}

} // namespace glu::gilgen
