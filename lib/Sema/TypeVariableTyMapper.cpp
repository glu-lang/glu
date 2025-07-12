#include "Basic/Diagnostic.hpp"
#include "ConstraintSystem.hpp"
#include "TyMapperVisitor.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

class TypeVariableTyMapper
    : public glu::sema::TypeMappingVisitorBase<TypeVariableTyMapper> {

    Solution *_solution;
    glu::DiagnosticManager &_diagManager;

public:
    using TypeMappingVisitorBase::TypeMappingVisitorBase;

    TypeVariableTyMapper(
        Solution *solution, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context
    )
        : TypeMappingVisitorBase(context)
        , _solution(solution)
        , _diagManager(diagManager)
    {
    }

    glu::types::TypeBase *visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        auto mapped = _solution->getTypeFor(type);
        if (!mapped) {
            _diagManager.error(
                SourceLocation::invalid, "Unresolved type variable"
            );
            return type;
        }
        return mapped;
    }
};

void ConstraintSystem::mapTypeVariables(SolutionResult &solutionRes)
{
    if (solutionRes.isAmbiguous()) {
        _diagManager.error(
            SourceLocation::invalid,
            "Ambiguous type variable mapping found, cannot resolve."
        );
        return;
    }

    Solution *solution = solutionRes.getBestSolution();

    if (!solution) {
        _diagManager.error(
            SourceLocation::invalid,
            "No best solution available for type variable mapping."
        );
        return;
    }
    TypeVariableTyMapper mapper(solution, _diagManager, _context);

    mapper.visit(_scopeTable->getGlobalScope()->getModule());
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
