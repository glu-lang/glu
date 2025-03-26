#ifndef GLU_GILGEN_GILGEN_HPP
#define GLU_GILGEN_GILGEN_HPP

#include "Decls.hpp"
#include "GIL/Function.hpp"

namespace glu::gilgen {

class GILGen {
public:
    GILGen() = default;
    ~GILGen() = default;

    gil::Function *
    generateFunction(ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena);
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGEN_HPP
