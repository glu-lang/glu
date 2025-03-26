#include "Context.hpp"
#include "Scope.hpp"

#include <vector>

namespace glu::gilgen {

using namespace glu::gilgen;
using namespace glu::ast;

class GILGenFunction {
    Context _context;
    std::vector<Scope> _scopes;

public:
    GILGenFunction(ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena)
        : _context(decl, arena), _scopes()
    {
        _scopes.push_back(decl);
    }
};

} // namespace glu::gilgen
