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
    gil::Module *module;
    llvm::BumpPtrAllocator &arena;
    llvm::DenseSet<ast::FunctionDecl *> _inlinableFunctions;

    GlobalContext(gil::Module *module, llvm::BumpPtrAllocator &arena)
        : module(module), arena(arena)
    {
    }
};

class GILGen {
public:
    GILGen() = default;
    ~GILGen() = default;

    gil::Global *getOrCreateGlobal(
        gil::Module *module, ast::VarLetDecl *decl,
        llvm::BumpPtrAllocator &arena
    );

    gil::Global *generateGlobal(
        gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
    );

    gil::Function *generateFunction(
        gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx
    );

    /// @brief Generate a GIL module from an AST module declaration
    /// @param moduleDecl The AST module declaration
    /// @param arena Memory allocator for GIL functions
    /// @param outFunctions Vector to store generated functions (allocated with
    /// arena)
    /// @return A new GIL module (functions stored separately due to memory
    /// management)
    gil::Module *
    generateModule(ast::ModuleDecl *moduleDecl, llvm::BumpPtrAllocator &arena);
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGEN_HPP
