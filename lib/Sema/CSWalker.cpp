#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "Sema/ConstraintSystem.hpp"

namespace glu::sema {

class LocalCSWalker : public glu::ast::ASTWalker<LocalCSWalker, void> {
    ConstraintSystem _cs;

public:
    LocalCSWalker(ScopeTable *scope) : _cs(scope) { }
};

class GlobalCSWalker : public ast::ASTWalker<GlobalCSWalker, void> {
    glu::sema::ScopeTable *_scopeTable;
    llvm::BumpPtrAllocator _allocator;

public:
    GlobalCSWalker() : _allocator() { }

    void preVisitModuleDecl(glu::ast::ModuleDecl *node)
    {
        _scopeTable = new (_allocator) glu::sema::ScopeTable(node);
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        _scopeTable = new (_allocator) glu::sema::ScopeTable(_scopeTable, node);
    }

    void visitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        LocalCSWalker(_scopeTable).visit(node);
    }

    void postVisitCompoundStmt([[maybe_unused]] glu::ast::CompoundStmt *node)
    {
        if (_scopeTable->getParent()) {
            _scopeTable = _scopeTable->getParent();
        }
    }

    void preVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable->insertItem(
            node->getBinding()->getName(), node->getBinding()
        );
    }

    void postVisitForStmt(glu::ast::ForStmt *node)
    {
        _scopeTable->removeItem(node->getBinding()->getName());
    }
};

void constrainAST(glu::ast::ModuleDecl *module)
{
    GlobalCSWalker().visit(module);
}

}
