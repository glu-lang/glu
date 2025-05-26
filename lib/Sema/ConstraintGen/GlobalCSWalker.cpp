#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "Sema/ConstraintSystem.hpp"

namespace glu::sema {

class GlobalCSWalker : public ast::ASTWalker<GlobalCSWalker, void> {

    glu::sema::ScopeTable *_scopeTable;

public:
    GlobalCSWalker(glu::sema::ScopeTable *scopeTable) : _scopeTable(scopeTable)
    {
    }

    void beforeVisitNode(glu::ast::ASTNode *node) { }

    void preVisitModuleDecl(glu::ast::ModuleDecl *node) {
    }
};

// Constraint *ConstraintSystem::genConstraints(glu::ast::DeclBase
// *parentModule)
// {
//     return GlobalCSWalker(this).visit(parentModule);
// }

}
