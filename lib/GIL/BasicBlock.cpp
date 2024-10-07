#include "BasicBlock.hpp"
#include <algorithm>
#include <iterator>

namespace glu::gil {

InstBase *BasicBlock::popFirstInstruction()
{
    if (!_instructions.empty()) {
        InstBase *inst = &_instructions.front();
        _instructions.remove(inst);
        return inst;
    }
    return nullptr;
}

void BasicBlock::addInstructionAtEnd(InstBase *inst)
{
    _instructions.push_back(inst);
}

void BasicBlock::addInstructionAtStart(InstBase *inst)
{
    _instructions.push_front(inst);
}

void BasicBlock::addInstructionAt(InstBase *inst, InstListType::iterator it)
{
    _instructions.insert(it, inst);
}

void BasicBlock::addInstructionBefore(InstBase *inst, InstBase *before)
{
    if (before) {
        assert(before->getParent() == this && "InstBase Parent mismatch");

        _instructions.insert(before->getIterator(), inst);
    } else {
        _instructions.push_front(inst);
    }
}

void BasicBlock::addInstructionAfter(InstBase *inst, InstBase *after)
{
    if (after) {
        assert(after->getParent() == this && "InstBase Parent mismatch");

        _instructions.insertAfter(after->getIterator(), inst);
    } else {
        _instructions.push_back(inst);
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

InstBase const *BasicBlock::getTerminator() const
{
    if (_instructions.empty() /*|| !_instructions.back().isTerminator()*/)
        return nullptr;
    return &_instructions.back();
}

InstBase *BasicBlock::getTerminator()
{
    return const_cast<InstBase *>(
        static_cast<BasicBlock const *>(this)->getTerminator()
    );
}

TerminatorInst *BasicBlock::getTerminatorInst() const
{
    return const_cast<TerminatorInst *>(
        static_cast<TerminatorInst const *>(getTerminator())
    );
}

void BasicBlock::setTerminator(InstBase *terminator)
{
    InstBase *lastInst = getTerminator();
    bool lastInstIsTerminator = lastInst != nullptr;

    if (lastInstIsTerminator) {
        replaceInstruction(lastInst, terminator);
    } else {
        addInstructionAtEnd(terminator);
    }
}

} // end namespace glu::gil

// ! TODO: implementation to move in InstBase file
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
