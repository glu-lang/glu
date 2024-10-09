#include "InstBase.hpp"
#include "InstVisitor.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::gil {

llvm::StringRef InstBase::getInstName() const
{
    switch (getKind()) {
#define GIL_INSTRUCTION(CLS, NAME, PARENT) \
    case InstKind::CLS##Kind: return NAME;
#include "InstKind.def"
    default: llvm_unreachable("Unknown instruction kind");
    }
}

BasicBlock *Value::getDefiningBlock() const
{
    if (auto block = value.dyn_cast<BasicBlock *>()) {
        return block;
    }
    return value.get<InstBase *>()->getParent();
}

} // namespace glu::gil
