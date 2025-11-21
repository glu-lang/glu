#ifndef GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
#define GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP

#include "AST/Decl/TemplateParameterDecl.hpp"
#include "Basic/Diagnostic.hpp"
#include "ScopeTable.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <memory>
#include <vector>

namespace glu::sema {

class UnresolvedNameTyMapper
    : public glu::sema::TypeMappingVisitorBase<UnresolvedNameTyMapper> {

    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;
    std::vector<std::unique_ptr<ScopeTable>> _ownedScopes;
    llvm::SmallVector<ScopeTable *, 4> _scopeStack;

public:
    using TypeMappingVisitorBase::TypeMappingVisitorBase;

    UnresolvedNameTyMapper(
        ScopeTable &globalScopeTable, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context
    )
        : TypeMappingVisitorBase(context)
        , _scopeTable(&globalScopeTable)
        , _diagManager(diagManager)
    {
    }

    // Handle namespace scopes when visiting NamespaceDecl nodes

    void preVisitNamespaceDecl(glu::ast::NamespaceDecl *decl)
    {
        TypeMappingVisitorBase::preVisitNamespaceDecl(decl);
        _scopeTable = _scopeTable->getLocalNamespace(decl->getName());
    }

    // overriding this is safe as there is no NODE_TYPEREF in NamespaceDecl
    void postVisitNamespaceDecl([[maybe_unused]] glu::ast::NamespaceDecl *decl)
    {
        TypeMappingVisitorBase::postVisitNamespaceDecl(decl);
        _scopeTable = _scopeTable->getParent();
    }

    void pushTemplateScope(
        glu::ast::TemplateParameterList *params, glu::ast::ASTNode *owner
    )
    {
        if (!_scopeTable || !params)
            return;

        auto local = std::make_unique<ScopeTable>(_scopeTable, owner);
        local->insertTemplateParams(params);
        _scopeStack.push_back(_scopeTable);
        _scopeTable = local.get();
        _ownedScopes.push_back(std::move(local));
    }

    void popTemplateScope(glu::ast::TemplateParameterList *params)
    {
        if (!params || _scopeStack.empty() || _ownedScopes.empty())
            return;
        _scopeTable = _scopeStack.pop_back_val();
        _ownedScopes.pop_back();
    }

    void preVisitFunctionDecl(glu::ast::FunctionDecl *decl)
    {
        TypeMappingVisitorBase::preVisitFunctionDecl(decl);
        pushTemplateScope(decl->getTemplateParams(), decl);
    }

    void postVisitFunctionDecl(glu::ast::FunctionDecl *decl)
    {
        TypeMappingVisitorBase::postVisitFunctionDecl(decl);
        popTemplateScope(decl->getTemplateParams());
    }

    void preVisitStructDecl(glu::ast::StructDecl *decl)
    {
        TypeMappingVisitorBase::preVisitStructDecl(decl);
        pushTemplateScope(decl->getTemplateParams(), decl);
    }

    void postVisitStructDecl(glu::ast::StructDecl *decl)
    {
        TypeMappingVisitorBase::postVisitStructDecl(decl);
        popTemplateScope(decl->getTemplateParams());
    }

    void preVisitTypeAliasDecl(glu::ast::TypeAliasDecl *decl)
    {
        TypeMappingVisitorBase::preVisitTypeAliasDecl(decl);
        pushTemplateScope(decl->getTemplateParams(), decl);
    }

    void postVisitTypeAliasDecl(glu::ast::TypeAliasDecl *decl)
    {
        TypeMappingVisitorBase::postVisitTypeAliasDecl(decl);
        popTemplateScope(decl->getTemplateParams());
    }

    glu::types::TypeBase *
    visitUnresolvedNameTy(glu::types::UnresolvedNameTy *type)
    {
        if (!_scopeTable) {
            // ScopeTable will only be null when an error has already occurred
            return type;
        }
        auto *item = _scopeTable->lookupType(type->getIdentifiers());
        if (!item) {
            _diagManager.error(
                type->getLocation(),
                "Unresolved type name '" + type->getIdentifiers().toString()
                    + "'"
            );
            return type; // Return unchanged so type checking can fail
                         // gracefully later
        }

        return item;
    }
};

}

#endif // GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
