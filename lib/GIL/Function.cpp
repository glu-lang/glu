#include "Module.hpp"

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

namespace llvm {
glu::gil::Module *ilist_traits<glu::gil::Function>::getContainingModule()
{
    size_t Offset = reinterpret_cast<size_t>(
        &((glu::gil::Module *) nullptr
              ->*glu::gil::Module::getSublistAccess(
                  static_cast<glu::gil::Function *>(nullptr)
              ))
    );
    iplist<glu::gil::Function, ilist_parent<glu::gil::Module>> *Anchor
        = static_cast<
            iplist<glu::gil::Function, ilist_parent<glu::gil::Module>> *>(this);
    return reinterpret_cast<glu::gil::Module *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}

glu::gil::Module *ilist_traits<glu::gil::Global>::getContainingModule()
{
    size_t Offset = reinterpret_cast<size_t>(
        &((glu::gil::Module *) nullptr
              ->*glu::gil::Module::getSublistAccess(
                  static_cast<glu::gil::Global *>(nullptr)
              ))
    );
    iplist<glu::gil::Global, ilist_parent<glu::gil::Module>> *Anchor
        = static_cast<
            iplist<glu::gil::Global, ilist_parent<glu::gil::Module>> *>(this);
    return reinterpret_cast<glu::gil::Module *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}
} // end namespace llvm
