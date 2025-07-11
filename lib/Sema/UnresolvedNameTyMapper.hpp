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
    glu::DiagnosticManager &_diagManager;

public:
    using TypeMappingVisitorBase::TypeMappingVisitorBase;

    UnresolvedNameTyMapper(
        ScopeTable &globalScopeTable, glu::DiagnosticManager &diagManager,
        InternedMemoryArena<types::TypeBase> &typesMemoryArena
    )
        : TypeMappingVisitorBase(typesMemoryArena)
        , _globalScopeTable(globalScopeTable)
        , _diagManager(diagManager)
    {
    }

    glu::types::TypeBase *
    visitUnresolvedNameTy(glu::types::UnresolvedNameTy *type)
    {
        llvm::StringRef name = type->getName();

        auto *item = _globalScopeTable.lookupType(name);
        if (!item) {
            _diagManager.error(
                item->getLocation(), "Unresolved type name '" + name.str() + "'"
            );
            return type; // Return unchanged so type checking can fail
                         // gracefully later
        }

        return item->getType();
    }
};

}

#endif // GLU_SEMA_UNRESOLVED_NAME_TY_MAPPER_HPP