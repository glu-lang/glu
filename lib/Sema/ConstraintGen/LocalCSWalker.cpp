#include "AST/ASTWalker.hpp"
#include "ConstraintSystem.hpp"
#include "ScopeTable.hpp"

namespace glu::sema {

class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem cs;

public:
    LocalCSWalker(ScopeTable *scopeTable) : cs(scopeTable) { }

    
};

}
