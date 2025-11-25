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

namespace glu::sema {

class UnresolvedNameTyMapper
    : public glu::sema::TypeMappingVisitorBase<UnresolvedNameTyMapper> {

    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;
    llvm::SmallVector<std::unique_ptr<ScopeTable>, 4> _scopeStack;

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
        _scopeTable = local.get();
        _scopeStack.push_back(std::move(local));
    }

    void popTemplateScope(glu::ast::TemplateParameterList *params)
    {
        if (!params || _scopeStack.empty())
            return;
        _scopeTable = _scopeStack.back()->getParent();
        _scopeStack.pop_back();
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
