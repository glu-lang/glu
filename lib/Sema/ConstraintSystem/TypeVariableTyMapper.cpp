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

    void _visitForStmt(glu::ast::ForStmt *node)
    {
        if (auto *binding = node->getBinding()) {
            this->visit(binding);
        }
        if (auto *range = node->getRange()) {
            this->visit(range);
        }
        auto visitRef = [this](glu::ast::RefExpr *ref) {
            if (ref) {
                this->visit(ref);
            }
        };
        visitRef(node->getBeginFunc());
        visitRef(node->getEndFunc());
        visitRef(node->getNextFunc());
        visitRef(node->getDerefFunc());
        visitRef(node->getEqualityFunc());
        // Body statements are handled separately by their own constraint
        // systems, so we intentionally skip node->getBody().
    }
};

void ConstraintSystem::mapTypeVariables(Solution *solution)
{
    TypeVariableTyMapper mapper(solution, _diagManager, _context);

    mapper.visit(_root);
}

} // namespace glu::sema
