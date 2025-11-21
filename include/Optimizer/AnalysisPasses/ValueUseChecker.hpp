#ifndef OPTIMIZER_ANALYSISPASSES_VALUEUSECHECKER_HPP
#define OPTIMIZER_ANALYSISPASSES_VALUEUSECHECKER_HPP

#include "GIL/InstVisitor.hpp"

namespace glu::optimizer {

bool valueIsUsedOnlyBy(glu::gil::Value value, glu::gil::InstBase *user);

}

#endif // OPTIMIZER_ANALYSISPASSES_VALUEUSECHECKER_HPP
