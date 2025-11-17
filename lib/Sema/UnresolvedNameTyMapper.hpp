#ifndef GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
#define GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP

#include "Basic/Diagnostic.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

class UnresolvedNameTyMapper
    : public glu::sema::TypeMappingVisitorBase<UnresolvedNameTyMapper> {

    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;

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
        _scopeTable = _scopeTable->getLocalNamespace(decl->getName());
    }

    // overriding this is safe as there is no NODE_TYPEREF in NamespaceDecl
    void postVisitNamespaceDecl([[maybe_unused]] glu::ast::NamespaceDecl *decl)
    {
        _scopeTable = _scopeTable->getParent();
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
