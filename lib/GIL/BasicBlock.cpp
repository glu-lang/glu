#include "BasicBlock.hpp"
#include <algorithm>
#include <iterator>

namespace glu::gil {

BasicBlock::BasicBlock()
{
}

BasicBlock::BasicBlock(std::string label)
    : _label(label)
{
}

llvm::ilist<InstBase> const &BasicBlock::getInstructions() const
{
    return _instructions;
}

std::size_t BasicBlock::getInstructionCount() const
{
    return _instructions.size();
}

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
    inst->parent = this;
    _instructions.push_back(inst);
}

void BasicBlock::addInstructionAtStart(InstBase *inst)
{
    inst->parent = this;
    _instructions.push_front(inst);
}

void BasicBlock::addInstructionAt(
    InstBase *inst, llvm::ilist<InstBase>::iterator it
)
{
    inst->parent = this;
    _instructions.insert(it, inst);
}

void BasicBlock::addInstructionBefore(InstBase *inst, InstBase *before)
{
    if (before) {
        assert(before->getParent() == this && "InstBase parent mismatch");
        auto it = before->getIterator();
        inst->parent = this;
        _instructions.insert(it, inst);
    } else {
        _instructions.push_front(inst);
    }
}

void BasicBlock::addInstructionAfter(InstBase *inst, InstBase *after)
{
    if (after) {
        assert(after->getParent() == this && "InstBase parent mismatch");
        auto it = after->getIterator();
        inst->parent = this;
        _instructions.insertAfter(it, inst);
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
    newInst->parent = this;
}

void BasicBlock::removeInstruction(InstBase *inst)
{
    assert(inst->getParent() == this && "InstBase parent mismatch");
    _instructions.remove(inst);
}

InstBase *BasicBlock::getTerminator() const
{
    return nullptr;
}

void BasicBlock::setLabel(std::string label)
{
    _label = label;
}

std::string const &BasicBlock::getLabel() const
{
    return _label;
}

} // namespace glu::gil
