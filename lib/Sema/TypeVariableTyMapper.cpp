#include "Basic/Diagnostic.hpp"
#include "ConstraintSystem.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {

class TypeVariableTyMapper : public glu::sema::TypeMapper<TypeVariableTyMapper>,
                             public glu::types::TypeVisitor<
                                 TypeVariableTyMapper, glu::types::TypeBase *> {
    glu::DiagnosticManager &_diagManager;
    Solution *_solution;

public:
    TypeVariableTyMapper(
        Solution *solution, glu::DiagnosticManager &diagManager
    )
        : _diagManager(diagManager), _solution(solution)
    {
    }

    glu::types::TypeBase *visitTypeBase(glu::types::TypeBase *type)
    {
        return type;
    }

    glu::types::TypeBase *visitTypeVariableTy(glu::types::TypeVariableTy *type)
    {
        auto mappedType = _solution->getTypeFor(type);
        if (!mappedType) {
            _diagManager.error(
                SourceLocation::invalid,
                "Type variable mapping not found for: " + type->getKind()
            );
            return type; // Return the original type if no mapping is found
        }
        return mappedType;
    }

    using glu::types::TypeVisitor<
        TypeVariableTyMapper, glu::types::TypeBase *>::visit;
    using glu::sema::TypeMapper<TypeVariableTyMapper>::visit;

    glu::types::TypeBase *mapType(glu::types::TypeBase *type)
    {
        return visit(type);
    }
};

} // namespace glu::sema
