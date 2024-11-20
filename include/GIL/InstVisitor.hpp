
#ifndef GLU_GIL_INSTVISITOR_HPP
#define GLU_GIL_INSTVISITOR_HPP

#include "Instructions.hpp"

namespace glu::gil {

template <typename Impl, typename RetTy = void, typename... ArgTys>
class InstVisitor {
    Impl *asImpl() { return static_cast<Impl *>(this); }

public:
    // TODO: Add support for visiting basic blocks and functions.

    /// @brief Visit an instruction.
    ///
    /// This function dispatches to the visit method for the instruction's
    /// kind.
    ///
    /// @param inst The instruction to visit.
    /// @param ...args Additional arguments to pass to the visitor.
    /// @return The value returned by the visitor.
    RetTy visit(InstBase *inst, ArgTys... args)
    {
        return _visitInst(inst, std::forward<ArgTys>(args)...);
    }

    RetTy _visitInst(InstBase *inst, ArgTys... args)
    {
        switch (inst->getKind()) {
#define GIL_INSTRUCTION(CLS, NAME, PARENT)                            \
case InstKind::CLS##Kind:                                             \
    return asImpl()->visit##CLS(inst, std::forward<ArgTys>(args)...);
#include "InstKind.def"
        default: llvm_unreachable("Unknown instruction kind");
        }
    }

    RetTy visitInstBase(InstBase *inst, ArgTys... args) { }

// FIXME: Replace InstBase as argument with CLS. Add static cast in switch
// above (when the classes are implemented)
#define GIL_INSTRUCTION(CLS, NAME, PARENT)                                   \
    RetTy visit##CLS(InstBase *inst, ArgTys... args)                         \
    {                                                                        \
        return asImpl()->visit##PARENT(inst, std::forward<ArgTys>(args)...); \
    }
#define GIL_INSTRUCTION_SUPER(CLS, PARENT) GIL_INSTRUCTION(CLS, "", PARENT)
#include "InstKind.def"
};

} // namespace glu::gil

#endif // GLU_GIL_INSTVISITOR_HPP
