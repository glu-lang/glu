#include "Basic/Diagnostic.hpp"
#include "ConstraintSystem.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

class TypeVariableTyMapper
    : public glu::sema::TypeMappingVisitorBase<TypeVariableTyMapper> {

    Solution *_solution;
    glu::DiagnosticManager &_diagManager;
    glu::ast::ASTContext *_context;

public:
    using TypeMappingVisitorBase::TypeMappingVisitorBase;

    TypeVariableTyMapper(
        Solution *solution, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context
    )
        : TypeMappingVisitorBase(context)
        , _solution(solution)
        , _diagManager(diagManager)
        , _context(context)
    {
    }

    glu::types::TypeBase *visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        auto mapped = substitute(type, _solution->typeBindings, _context);
        if (llvm::isa<glu::types::TypeVariableTy>(mapped)) {
            _diagManager.error(_location, "Unresolved type variable");
        }
        return mapped;
    }
};

void ConstraintSystem::mapTypeVariables(Solution *solution)
{
    TypeVariableTyMapper mapper(solution, _diagManager, _context);

    mapper.visit(_scopeTable->getNode());
}

void ConstraintSystem::mapTypeVariablesToExpressions(
    Solution *solution, llvm::ArrayRef<glu::ast::ExprBase *> expressions
)
{
    if (!solution) {
        _diagManager.error(
            SourceLocation::invalid,
            "No solution available for type variable mapping."
        );
        return;
    }

    TypeVariableTyMapper mapper(solution, _diagManager, _context);

    // Apply type mapping directly to each expression
    for (auto *expr : expressions) {
        if (expr && expr->getType()) {
            auto *mappedType = mapper.visit(expr->getType());
            expr->setType(mappedType);
        }
    }
}

} // namespace glu::sema
