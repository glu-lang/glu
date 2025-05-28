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

    void postVisitModuleDecl([[maybe_unused]] glu::ast::ModuleDecl *node)
    {
        _scopeTable.pop_back();
    }

    void preVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitFunctionDecl([[maybe_unused]] glu::ast::FunctionDecl *node)
    {
        _scopeTable.pop_back();
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitCompoundStmt([[maybe_unused]] glu::ast::CompoundStmt *node)
    {
        assert(_scopeTable.back().getParent() && "Cannot pop global scope");
        _scopeTable.pop_back();
    }

    void preVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable.push_back(ScopeTable(&_scopeTable.back(), node));
    }

    void postVisitForStmt([[maybe_unused]] glu::ast::ForStmt *node)
    {
        _scopeTable.pop_back();
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
