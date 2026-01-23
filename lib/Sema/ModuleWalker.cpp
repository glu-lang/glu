#include "AST/ASTWalker.hpp"
#include "Sema.hpp"

#include "ImportManager.hpp"
#include "UnresolvedNameTyMapper.hpp"

#include "SemanticPass/DuplicateFunctionChecker.hpp"
#include "SemanticPass/EnumValueResolver.hpp"
#include "SemanticPass/ImmutableAssignmentWalker.hpp"
#include "SemanticPass/ImplementImportChecker.hpp"
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

namespace glu::sema {

/// @brief Walks the AST to build scope tables and run local constraint
/// systems. Runs the whole Sema pipeline.
class ModuleWalker : public glu::ast::ASTWalker<ModuleWalker, void> {
    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;
    glu::ast::ASTContext *_context;
    ImportManager *_importManager;
    llvm::SpecificBumpPtrAllocator<ScopeTable> &_globalScopeAllocator;
    llvm::SpecificBumpPtrAllocator<ScopeTable> _localScopeAllocator;
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
        , _globalScopeAllocator(importManager->getScopeTableAllocator())
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
        _scopeTable = new (_globalScopeAllocator.Allocate())
            ScopeTable(node, _importManager, _skipBodies);
        UnresolvedNameTyMapper mapper(*_scopeTable, _diagManager, _context);

        mapper.visit(node);
    }

    void postVisitModuleDecl([[maybe_unused]] glu::ast::ModuleDecl *node)
    {
        // Link drop/copy functions to their struct types (needed for all
        // modules)
        ValidDropOverloadChecker(_diagManager).visit(node);
        ValidCopyOverloadChecker(_diagManager).visit(node);

        if (!_skipBodies) {
            // These checks don't need to run on imported modules
            InitializerWalker(_diagManager).visit(node);
            ValidAttributeChecker(_diagManager).visit(node);
            ValidMainChecker(_diagManager).visit(node);
            DuplicateFunctionChecker(_diagManager).visit(node);
            InvalidOperatorArgsChecker(_diagManager).visit(node);
            ImplementImportChecker(*_importManager, _scopeTable, node)
                .process();

            // Process synthetic functions through Sema to resolve their
            // unresolved references (e.g., calls to local implementations)
            for (auto *synthetic : _scopeTable->getSyntheticFunctions()) {
                visit(synthetic);
            }
        }
    }

    void preVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        if (shouldSkipFunction(node)) {
            _skippingCurrentFunction = true;
            return;
        }
        _scopeTable = new (_localScopeAllocator.Allocate())
            ScopeTable(_scopeTable, node);
        _scopeTable->insertTemplateParams(node->getTemplateParams());
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
        _localScopeAllocator.DestroyAll();
    }

    void postVisitEnumDecl(glu::ast::EnumDecl *node)
    {
        ValidTypeChecker(_diagManager).visit(node);
        EnumValueResolver(_diagManager).visit(node);
    }

    void preVisitStructDecl(glu::ast::StructDecl *node)
    {
        if (!_scopeTable || !node->getTemplateParams())
            return;
        _scopeTable = new (_localScopeAllocator.Allocate())
            ScopeTable(_scopeTable, node);
        _scopeTable->insertTemplateParams(node->getTemplateParams());
    }

    void postVisitStructDecl(glu::ast::StructDecl *node)
    {
        if (!node->getTemplateParams() || !_scopeTable)
            return;
        _scopeTable = _scopeTable->getParent();
    }

    void preVisitTypeAliasDecl(glu::ast::TypeAliasDecl *node)
    {
        if (!_scopeTable || !node->getTemplateParams())
            return;
        _scopeTable = new (_localScopeAllocator.Allocate())
            ScopeTable(_scopeTable, node);
        _scopeTable->insertTemplateParams(node->getTemplateParams());
    }

    void postVisitTypeAliasDecl(glu::ast::TypeAliasDecl *node)
    {
        if (!node->getTemplateParams() || !_scopeTable)
            return;
        _scopeTable = _scopeTable->getParent();
    }

    void _visitNamespaceDecl(glu::ast::NamespaceDecl *node)
    {
        auto *namespaceScope = _scopeTable->getLocalNamespace(node->getName());
        assert(namespaceScope && "Namespace scope must exist in current scope");
        bool skip = false; //< In case of error, skip entering namespace
        if (namespaceScope->getParent() != _scopeTable) {
            _diagManager.error(
                node->getLocation(),
                llvm::Twine("Local namespace '") + node->getName()
                    + "' conflicts with an imported namespace"
            );
            skip = true;
        }
        if (!skip) {
            _scopeTable = namespaceScope;
        }
        for (auto *decl : node->getDecls()) {
            visit(decl);
        }
        if (!skip) {
            _scopeTable = _scopeTable->getParent();
        }
    }

    void preVisitCompoundStmt(glu::ast::CompoundStmt *node)
    {
        if (_skippingCurrentFunction)
            return;
        _scopeTable = new (_localScopeAllocator.Allocate())
            ScopeTable(_scopeTable, node);
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
        preVisitStmtBase(node);
        _scopeTable = new (_localScopeAllocator.Allocate())
            ScopeTable(_scopeTable, node);
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

    void postVisitParamDecl(glu::ast::ParamDecl *node)
    {
        // Run constraint system on parameter default values
        if (node->getValue()) {
            ScopeTable local(_scopeTable, node);
            runLocalCSWalker(
                &local, node, _diagManager, _context, _dumpConstraints
            );
        }
        // Call parent class handler for VarLetDecl
        postVisitVarLetDecl(node);
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

ScopeTable *constrainAST(
    glu::ast::ModuleDecl *module, glu::DiagnosticManager &diagManager,
    ImportManager *importManager, bool dumpConstraints
)
{
    ModuleWalker walker(
        diagManager, module->getContext(), importManager, dumpConstraints
    );
    walker.visit(module);
    return walker.getScopeTable();
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
