#include "BasicBlock.hpp"
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

void InstBase::eraseFromParent()
{
    assert(parent && "Instruction has no parent basic block");
    parent->removeInstruction(this);
    parent = nullptr;
}

} // end namespace glu::gil

namespace llvm {
glu::gil::BasicBlock *ilist_traits<glu::gil::InstBase>::getContainingBlock()
{
    size_t Offset = reinterpret_cast<size_t>(
        &((glu::gil::BasicBlock *) nullptr
              ->*glu::gil::BasicBlock::getSublistAccess(
                  static_cast<glu::gil::InstBase *>(nullptr)
              ))
    );
    iplist<glu::gil::InstBase> *Anchor
        = static_cast<iplist<glu::gil::InstBase> *>(this);
    return reinterpret_cast<glu::gil::BasicBlock *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}
} // end namespace llvm
