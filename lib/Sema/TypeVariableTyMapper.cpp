#include "ConstraintSystem.hpp"
#include "TypeMapper.hpp"

namespace glu::sema {
class TypeVariableTyMapper
    : public glu::sema::TypeMapper<TypeVariableTyMapper> {
    SolutionResult &result;

public:
    TypeVariableTyMapper(SolutionResult &result) : result(result) {}

    
};

} // namespace glu::sema
