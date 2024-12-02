#ifndef GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
#define GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP

#include "InstBase.hpp"

namespace glu::gil {

/**
 * @class AllocaInst
 * @brief Represents an allocation instruction.
 *
 * This class is derived from InstBase and represents an allocation instruction
 * in the GLU GIL (Generic Intermediate Language).
 */
class AllocaInst : public InstBase {
    size_t size; ///< The size of the allocation.
    Type type; ///< The type of the allocation.
public:
    /**
     * @brief Constructs an AllocaInst object.
     *
     * @param size The size of the allocation.
     * @param type The type of the allocation.
     */
    AllocaInst(size_t size, Type type)
        : InstBase(InstKind::AllocaInstKind), size(size), type(type)
    {
    }

    /**
     * @brief Gets the size of the allocation.
     *
     * @return The size of the allocation.
     */
    size_t getSize() const { return size; }

    /**
     * @brief Gets the type of the allocation.
     *
     * @return The type of the allocation.
     */
    Type getType() const { return type; }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
