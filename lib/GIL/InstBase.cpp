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
    auto *parentBlock = getParent();
    assert(parentBlock && "Instruction has no parent basic block");
    parentBlock->removeInstruction(this);
}

size_t InstBase::getResultCount() const
{
    switch (getKind()) {
#define GIL_RESULT_SINGLE(CLS) 1
#define GIL_RESULT_NONE(CLS) 0
#define GIL_RESULT_MULTIPLE(CLS) llvm::cast<CLS>(this)->getResultCountImpl()
#define GIL_INSTRUCTION_(CLS, NAME, PARENT, RESULT, ...) \
    case InstKind::CLS##Kind: return RESULT(CLS);
#include "InstKind.def"
    default: llvm_unreachable("Unknown instruction kind");
    }
}

Type InstBase::getResultType(size_t index) const
{
    switch (getKind()) {
#define GIL_RESULT_SINGLE(CLS) return llvm::cast<CLS>(this)->getResultType()
#define GIL_RESULT_NONE(CLS) llvm_unreachable("Invalid index")
#define GIL_RESULT_MULTIPLE(CLS)                           \
    return llvm::cast<CLS>(this)->getResultTypeImpl(index)
#define GIL_INSTRUCTION_(CLS, NAME, PARENT, RESULT, ...) \
    case InstKind::CLS##Kind: RESULT(CLS);
#include "InstKind.def"
    default: llvm_unreachable("Unknown instruction kind");
    }
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
    iplist<glu::gil::InstBase, ilist_parent<glu::gil::BasicBlock>> *Anchor
        = static_cast<
            iplist<glu::gil::InstBase, ilist_parent<glu::gil::BasicBlock>> *>(
            this
        );
    return reinterpret_cast<glu::gil::BasicBlock *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}

} // end namespace llvm
