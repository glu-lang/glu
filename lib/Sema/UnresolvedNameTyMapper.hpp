#ifndef GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP
#define GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP

#include "Basic/Diagnostic.hpp"
#include "ConstraintSystem.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

class UnresolvedNameTyMapper
    : public glu::sema::TypeMappingVisitorBase<UnresolvedNameTyMapper> {

    ScopeTable &_globalScopeTable;

public:
    using TypeMappingVisitorBase::TypeMappingVisitorBase;

    UnresolvedNameTyMapper(
        ScopeTable &globalScopeTable, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context
    )
        : TypeMappingVisitorBase(diagManager, context)
        , _globalScopeTable(globalScopeTable)
    {
    }

    glu::types::TypeBase *
    visitUnresolvedNameTy(glu::types::UnresolvedNameTy *type)
    {
        llvm::StringRef name = type->getName();

        // Step 1: lookup type declaration directly
        auto *item = _globalScopeTable.lookupItem(name);
        if (!item || item->decls.empty()) {
            _diagManager.error(
                SourceLocation::invalid,
                "Unresolved type name '" + name.str() + "'"
            );
            return type; // Return unchanged so type checking can fail
                         // gracefully later
        }

        // Step 2: Expect the first (or only) declaration to be a TypeDecl
        if (auto *typeDecl
            = llvm::dyn_cast<glu::ast::TypeDecl>(item->decls.front())) {
            return typeDecl->getType();
        }

        _diagManager.error(
            SourceLocation::invalid,
            "Identifier '" + name.str() + "' does not refer to a type"
        );
        return type;
    }
};

}

#endif // GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP