#include "AST/ASTWalker.hpp"
#include "Sema.hpp"

#include "ImportManager.hpp"
#include "UnresolvedNameTyMapper.hpp"

#include "SemanticPass/DuplicateFunctionChecker.hpp"
#include "SemanticPass/ImmutableAssignmentWalker.hpp"
#include "SemanticPass/InitializerWalker.hpp"
#include "SemanticPass/InvalidOperatorArgsChecker.hpp"
#include "SemanticPass/UnreachableWalker.hpp"
#include "SemanticPass/UnreferencedVarDeclWalker.hpp"
#include "SemanticPass/ValidAttributeChecker.hpp"
#include "SemanticPass/ValidCopyOverloadChecker.hpp"
#include "SemanticPass/ValidDropOverloadChecker.hpp"
#include "SemanticPass/ValidLiteralChecker.hpp"
#include "SemanticPass/ValidMainChecker.hpp"
#include "SemanticPass/ValidTypeChecker.hpp"

#include <llvm/Support/WithColor.h>

#include <variant>

namespace glu::sema {

/// @brief Walks the AST to build scope tables and run local constraint
/// systems. Runs the whole Sema pipeline.
class ModuleWalker : public glu::ast::ASTWalker<ModuleWalker, void> {
    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;
    glu::ast::ASTContext *_context;
    ImportManager *_importManager;
    llvm::BumpPtrAllocator &_scopeTableAllocator;
    bool _skipBodies = false;
    bool _skippingCurrentFunction = false;

    llvm::raw_ostream *_dumpConstraints
        = nullptr; ///< Whether to dump constraints

public:
    ModuleWalker(
        glu::DiagnosticManager &diagManager, glu::ast::ASTContext *context,
        ImportManager *importManager, bool dumpConstraints = false
    )
        : _diagManager(diagManager)
        , _context(context)
        , _importManager(importManager)
        , _scopeTableAllocator(importManager->getScopeTableAllocator())
        , _dumpConstraints(dumpConstraints ? &llvm::outs() : nullptr)
    {
    }

    ScopeTable *getScopeTable() const { return _scopeTable; }
    void setSkipBodies(bool skip) { _skipBodies = skip; }

    bool shouldSkipFunction(glu::ast::FunctionDecl *node)
    {
        if (!_skipBodies)
            return false;
        if (node->hasAttribute(ast::AttributeKind::InlineKind))
            return false;
        return true;
    }

    void preVisitModuleDecl(glu::ast::ModuleDecl *node)
    {
        _scopeTable = new (_scopeTableAllocator)
            ScopeTable(node, _importManager, _skipBodies);
        UnresolvedNameTyMapper mapper(*_scopeTable, _diagManager, _context);

        mapper.visit(node);
    }

    void postVisitModuleDecl([[maybe_unused]] glu::ast::ModuleDecl *node)
    {
        if (!_skipBodies) {
            // These checks don't need to run on imported modules
            InitializerWalker(_diagManager).visit(node);
            ValidAttributeChecker(_diagManager).visit(node);
            ValidMainChecker(_diagManager).visit(node);
            DuplicateFunctionChecker(_diagManager).visit(node);
            InvalidOperatorArgsChecker(_diagManager).visit(node);
            ValidDropOverloadChecker(_diagManager).visit(node);
            ValidCopyOverloadChecker(_diagManager).visit(node);
        }
    }

    void preVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        if (shouldSkipFunction(node)) {
            _skippingCurrentFunction = true;
            return;
        }
        _scopeTable = new (_scopeTableAllocator) ScopeTable(_scopeTable, node);
    }

    void postVisitFunctionDecl([[maybe_unused]] glu::ast::FunctionDecl *node)
    {
        if (shouldSkipFunction(node)) {
            _skippingCurrentFunction = false;
            return;
        }
        UnreachableWalker(_diagManager).visit(node);
        UnreferencedVarDeclWalker(_diagManager).visit(node);
        ImmutableAssignmentWalker(_diagManager).visit(node);
        ValidLiteralChecker(_diagManager).visit(node);
        ValidTypeChecker(_diagManager).visit(node);
        _scopeTable = _scopeTable->getParent();
    }

    void postVisitEnumDecl(glu::ast::EnumDecl *node)
    {
        ValidTypeChecker(_diagManager).visit(node);
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        if (_skippingCurrentFunction)
            return;
        _scopeTable = new (_scopeTableAllocator) ScopeTable(_scopeTable, node);
    }

    void postVisitCompoundStmt([[maybe_unused]] glu::ast::CompoundStmt *node)
    {
        if (_skippingCurrentFunction)
            return;
        assert(_scopeTable->getParent() && "Cannot pop global scope");
        _scopeTable = _scopeTable->getParent();
    }

    void preVisitForStmt(glu::ast::ForStmt *node)
    {
        if (_skippingCurrentFunction)
            return;
        _scopeTable = new (_scopeTableAllocator) ScopeTable(_scopeTable, node);
    }

    void postVisitForStmt([[maybe_unused]] glu::ast::ForStmt *node)
    {
        if (_skippingCurrentFunction)
            return;
        _scopeTable = _scopeTable->getParent();
    }

    void postVisitVarLetDecl(glu::ast::VarLetDecl *node)
    {
        if (node->isGlobal())
            return; // Global variables are handled in the module scope table
        if (_skippingCurrentFunction)
            return;
        _scopeTable->insertItem(node->getName(), node, node->getVisibility());
    }

    void preVisitVarLetDecl(glu::ast::VarLetDecl *node)
    {
        if (node->isGlobal()) {
            ScopeTable local(_scopeTable, node);
            runLocalCSWalker(
                &local, node, _diagManager, _context, _dumpConstraints
            );
        }
    }

    void postVisitFieldDecl(glu::ast::FieldDecl *node)
    {
        if (llvm::isa<ast::StructDecl>(node->getParent())) {
            ScopeTable local(_scopeTable, node);
            runLocalCSWalker(
                &local, node, _diagManager, _context, _dumpConstraints
            );
        }
    }

    void preVisitStmtBase(glu::ast::StmtBase *node)
    {
        if (_skippingCurrentFunction)
            return;
        ScopeTable local(_scopeTable, node);
        runLocalCSWalker(
            &local, node, _diagManager, _context, _dumpConstraints
        );
    }
};

void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    llvm::ArrayRef<std::string> importPaths, bool dumpConstraints
)
{
    ImportManager importManager(
        *module->getContext(), diagManager, importPaths
    );
    constrainAST(module, diagManager, &importManager, dumpConstraints);
}

void constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    ImportManager *importManager, bool dumpConstraints
)
{
    ModuleWalker(
        diagManager, module->getContext(), importManager, dumpConstraints
    )
        .visit(module);
}

ScopeTable *fastConstrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    ImportManager *importManager
)
{
    ModuleWalker walker(diagManager, module->getContext(), importManager);
    walker.setSkipBodies(true);
    walker.visit(module);
    return walker.getScopeTable();
}

} // namespace glu::sema
