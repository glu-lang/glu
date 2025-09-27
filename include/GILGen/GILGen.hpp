#ifndef GLU_GILGEN_GILGEN_HPP
#define GLU_GILGEN_GILGEN_HPP

#include "Decls.hpp"
#include "GIL/Function.hpp"
#include "GIL/Module.hpp"

namespace glu::gilgen {

class GILGen {
public:
    GILGen() = default;
    ~GILGen() = default;

    gil::Global *getOrCreateGlobal(
        gil::Module *module, ast::VarLetDecl *decl,
        llvm::BumpPtrAllocator &arena
    );

    gil::Global *generateGlobal(
        gil::Module *module, ast::VarLetDecl *decl,
        llvm::BumpPtrAllocator &arena
    );

    gil::Function *generateFunction(
        gil::Module *module, ast::FunctionDecl *decl,
        llvm::BumpPtrAllocator &arena
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

    void runGILPasses(gil::Module *module);
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGEN_HPP
