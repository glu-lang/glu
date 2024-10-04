#include "InstBase.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::gil {

llvm::StringRef InstBase::getInstName() {
    switch (getKind()) {
    #define GIL_INSTRUCTION(CLS, NAME) \
    case InstKind::CLS ## Kind: \
        return NAME;
    #include "InstKind.def"
    #undef GIL_INSTRUCTION
    default:
        llvm_unreachable("Unknown instruction kind");
    }
}

BasicBlock *GILValue::getDefiningBlock() {
    if (auto block = value.dyn_cast<BasicBlock *>()) {
        return block;
    }
    return value.get<InstBase *>()->getParent();
}

} // namespace glu::gil