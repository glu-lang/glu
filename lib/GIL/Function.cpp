#include "Function.hpp"

namespace glu::gil {

void Function::addBasicBlockBefore(BasicBlock *bb, BasicBlock *before)
{
    if (before) {
        assert(before->getParent() == this && "BasicBlock Parent mismatch");

        _basicBlocks.insert(before->getIterator(), bb);
    } else {
        _basicBlocks.push_front(bb);
    }
}

void Function::addBasicBlockAfter(BasicBlock *bb, BasicBlock *after)
{
    if (after) {
        assert(after->getParent() == this && "BasicBlock Parent mismatch");

        _basicBlocks.insertAfter(after->getIterator(), bb);
    } else {
        _basicBlocks.push_back(bb);
    }
}

void Function::replaceBasicBlock(BasicBlock *oldBB, BasicBlock *newBB)
{
    assert(oldBB->getParent() == this && "BasicBlock parent mismatch");

    auto it = oldBB->getIterator();

    _basicBlocks.insert(it, newBB);
    _basicBlocks.erase(it);
}

} // end namespace glu::gil
