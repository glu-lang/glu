#ifndef GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
#define GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP

#include "AST/Decl/TemplateParameterDecl.hpp"
#include "Basic/Diagnostic.hpp"
#include "ScopeTable.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>

namespace glu::sema {

class UnresolvedNameTyMapper
    : public glu::sema::TypeMappingVisitorBase<UnresolvedNameTyMapper> {

    ScopeTable *_scopeTable;
    glu::DiagnosticManager &_diagManager;
    llvm::SmallVector<ast::TemplateParameterList *, 4> _templateParamStack;

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

    void preVisitASTNode(glu::ast::ASTNode *node)
    {
        TypeMappingVisitorBase::preVisitASTNode(node);
        if (auto *templated = llvm::dyn_cast<ast::FunctionDecl>(node)) {
            if (auto *params = templated->getTemplateParams())
                _templateParamStack.push_back(params);
        } else if (auto *templated = llvm::dyn_cast<ast::StructDecl>(node)) {
            if (auto *params = templated->getTemplateParams())
                _templateParamStack.push_back(params);
        } else if (auto *templated = llvm::dyn_cast<ast::TypeAliasDecl>(node)) {
            if (auto *params = templated->getTemplateParams())
                _templateParamStack.push_back(params);
        }
    }

    void postVisitASTNode(glu::ast::ASTNode *node)
    {
        if (auto *templated = llvm::dyn_cast<ast::TypeAliasDecl>(node)) {
            if (templated->getTemplateParams())
                _templateParamStack.pop_back();
        } else if (auto *templated = llvm::dyn_cast<ast::StructDecl>(node)) {
            if (templated->getTemplateParams())
                _templateParamStack.pop_back();
        } else if (auto *templated = llvm::dyn_cast<ast::FunctionDecl>(node)) {
            if (templated->getTemplateParams())
                _templateParamStack.pop_back();
        }
        TypeMappingVisitorBase::postVisitASTNode(node);
    }

    glu::types::TemplateParamTy *
    lookupTemplateParameter(ast::NamespaceIdentifier ident)
    {
        if (!ident.components.empty())
            return nullptr;
        for (auto it = _templateParamStack.rbegin();
             it != _templateParamStack.rend(); ++it) {
            for (auto *param : (*it)->getTemplateParameters()) {
                if (param->getName() == ident.identifier)
                    return param->getType();
            }
        }
        return nullptr;
    }

    glu::types::TypeBase *
    visitUnresolvedNameTy(glu::types::UnresolvedNameTy *type)
    {
        if (!_scopeTable) {
            // ScopeTable will only be null when an error has already occurred
            return type;
        }
        if (auto *tpl = lookupTemplateParameter(type->getIdentifiers()))
            return tpl;
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
