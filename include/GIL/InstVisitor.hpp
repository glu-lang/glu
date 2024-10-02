
#ifndef GLU_GIL_INSTVISITOR_HPP
#define GLU_GIL_INSTVISITOR_HPP

#include "InstBase.hpp"

namespace glu::gil {

class InstVisitor {
public:
    // TODO: Add support for visiting basic blocks and functions.
    void visit(InstBase *inst) {
        _visitInst(inst);
    }
    void _visitInst(InstBase *inst) {
        visitInstBase(inst);
        switch (inst->getKind()) {
        #define GIL_INSTRUCTION(CLS, NAME) \
        case InstKind::CLS ## Kind: \
            visit ## CLS (static_cast<CLS *>(inst)); \
            break;
        #include "InstKind.def"
        #undef GIL_INSTRUCTION
        default:
            llvm_unreachable("Unknown instruction kind");
        }
    }

    virtual void visitInstBase(InstBase *inst) {}

    #define GIL_INSTRUCTION(CLS, NAME) \
    virtual void visit ## CLS (CLS *inst) {}
    #include "InstKind.def"
    #undef GIL_INSTRUCTION

};

} // namespace glu::gil

#endif // GLU_GIL_INSTVISITOR_HPP