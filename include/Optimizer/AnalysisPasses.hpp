#ifndef GLU_OPTIMIZER_ANALYSIS_PASSES_HPP
#define GLU_OPTIMIZER_ANALYSIS_PASSES_HPP

#include "GIL/InstVisitor.hpp"

namespace glu::optimizer {

/// @brief Returns true if the given value is only referenced by the provided
/// instruction. Useful when deciding whether removing the user will orphan the
/// value or if other users still depend on it.
/// @param value The value to check
/// @param user The only allowed user of the value
/// @return true if the value is only used by the specified instruction, false
/// otherwise
bool valueIsUsedOnlyBy(glu::gil::Value value, glu::gil::InstBase *user);

} // namespace glu::optimizer

#endif // GLU_OPTIMIZER_ANALYSIS_PASSES_HPP
