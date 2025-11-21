#ifndef GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
#define GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP

#include "Basic/Diagnostic.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

#include <string>

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

        if (auto *structTy = llvm::dyn_cast<glu::types::StructTy>(item)) {
            auto *params = structTy->getDecl()->getTemplateParams();
            auto templateArgs = type->getTemplateArgs();
            size_t paramCount
                = params ? params->getTemplateParameters().size() : 0;

            if (paramCount == 0 && !templateArgs.empty()) {
                _diagManager.error(
                    type->getLocation(),
                    "Type '" + type->getIdentifiers().toString()
                        + "' does not take template arguments"
                );
                return type;
            }

            if (paramCount != templateArgs.size()) {
                _diagManager.error(
                    type->getLocation(),
                    "Type '" + type->getIdentifiers().toString() + "' expects "
                        + std::to_string(paramCount)
                        + " template argument(s) but got "
                        + std::to_string(templateArgs.size())
                );
                return type;
            }

            if (paramCount == 0) {
                return structTy; // Non-templated struct
            }

            return _types.create<glu::types::StructTy>(
                structTy->getDecl(), templateArgs
            );
        }

        return item;
    }
};

}

#endif // GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
