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

} // namespace glu::gil