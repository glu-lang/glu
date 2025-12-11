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

/// @brief Returns true if the given value is only referenced by the two
/// provided instructions. Useful for optimization passes that transform
/// patterns involving two specific uses of a value.
/// @param value The value to check
/// @param user1 The first allowed user of the value
/// @param user2 The second allowed user of the value
/// @return true if the value is only used by the two specified instructions,
/// false otherwise
bool valueIsUsedOnlyBy(
    glu::gil::Value value, glu::gil::InstBase *user1, glu::gil::InstBase *user2
);

/// @brief Returns true if the given instruction uses the specified value as an
/// operand. This checks all operands of the instruction.
/// @param inst The instruction to check
/// @param value The value to look for
/// @return true if the instruction uses the value, false otherwise
bool instructionUsesValue(glu::gil::InstBase *inst, glu::gil::Value value);

} // namespace glu::optimizer

#endif // GLU_OPTIMIZER_ANALYSIS_PASSES_HPP
