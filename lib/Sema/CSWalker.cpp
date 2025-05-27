#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "Sema/ConstraintSystem.hpp"

namespace glu::sema {

class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem _cs;

public:
    LocalCSWalker(ScopeTable *scope) : _cs(scope) { }
    ~LocalCSWalker() { _cs.resolveConstraints(); }
};

class GlobalCSWalker : public glu::ast::ASTWalker<GlobalCSWalker, void> {
    std::vector<ScopeTable> _scopeTable;

public:
    GlobalCSWalker() { }

    void preVisitModuleDecl(glu::ast::ModuleDecl *node)
    {
        _scopeTable.push_back(ScopeTable(node));
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitCompoundStmt([[maybe_unused]] glu::ast::CompoundStmt *node)
    {
        if (_scopeTable.back().getParent())
            _scopeTable.pop_back();
    }

    void preVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable.back().insertItem(
            node->getBinding()->getName(), node->getBinding()
        );
    }

    void postVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable.back().removeItem(node->getBinding()->getName());
    }

    void preVisitStmt(glu::ast::StmtBase *node)
    {
        LocalCSWalker(&_scopeTable.back()).visit(node);
    }
};

void constrainAST(glu::ast::ModuleDecl *module)
{
    GlobalCSWalker().visit(module);
}

}
