#include "Function.hpp"
#include <algorithm>
#include <iterator>

namespace glu::gil {

InstBase *BasicBlock::popFirstInstruction()
{
    if (_instructions.empty())
        return nullptr;
    InstBase *inst = &_instructions.front();
    _instructions.remove(inst);
    return inst;
}

void BasicBlock::addInstructionBefore(InstBase *inst, InstBase *before)
{
    if (before) {
        assert(before->getParent() == this && "InstBase Parent mismatch");

        _instructions.insert(before->getIterator(), inst);
    } else {
        _instructions.push_back(inst);
    }
}

void BasicBlock::addInstructionAfter(InstBase *inst, InstBase *after)
{
    if (after) {
        assert(after->getParent() == this && "InstBase Parent mismatch");

        _instructions.insertAfter(after->getIterator(), inst);
    } else {
        _instructions.push_front(inst);
    }
}

void BasicBlock::replaceInstruction(InstBase *oldInst, InstBase *newInst)
{
    assert(oldInst->getParent() == this && "InstBase parent mismatch");

    auto it = oldInst->getIterator();

    _instructions.insert(it, newInst);
    _instructions.erase(it);
}

void BasicBlock::removeInstruction(InstBase *inst)
{
    assert(inst->getParent() == this && "InstBase Parent mismatch");

    _instructions.remove(inst);
}

TerminatorInst *BasicBlock::getTerminator()
{
    if (_instructions.empty())
        return nullptr;
    return llvm::dyn_cast<TerminatorInst>(&_instructions.back());
}

void BasicBlock::setTerminator(TerminatorInst *terminator)
{
    if (TerminatorInst *lastInst = getTerminator()) {
        replaceInstruction(lastInst, terminator);
    } else {
        addInstructionAtEnd(terminator);
    }
}

} // end namespace glu::gil

namespace llvm {
glu::gil::Function *ilist_traits<glu::gil::BasicBlock>::getContainingFunction()
{
    size_t Offset = reinterpret_cast<size_t>(
        &((glu::gil::Function *) nullptr
              ->*glu::gil::Function::getSublistAccess(
                  static_cast<glu::gil::BasicBlock *>(nullptr)
              ))
    );
    iplist<glu::gil::BasicBlock, ilist_parent<glu::gil::Function>> *Anchor
        = static_cast<
            iplist<glu::gil::BasicBlock, ilist_parent<glu::gil::Function>> *>(
            this
        );
    return reinterpret_cast<glu::gil::Function *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}
} // end namespace llvm
