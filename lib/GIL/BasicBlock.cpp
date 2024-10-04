#include "BasicBlock.hpp"
#include <algorithm>
#include <iterator>

namespace glu::gil {

BasicBlock::BasicBlock()
{
}

BasicBlock::BasicBlock(
    std::string const &label, std::list<InstBase *> const &instructions
)
    : _instructions(std::move(instructions))
    , _label(std::move(label))
{
}

BasicBlock::BasicBlock(std::list<InstBase *> const &instructions)
    : _instructions(std::move(instructions))
{
}

std::list<InstBase *> BasicBlock::getInstructions() const
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
        auto inst = _instructions.front();
        _instructions.pop_front();
        return inst;
    }

    return nullptr;
}

InstBase *BasicBlock::getInstructionAt(std::size_t index) const
{
    if (index < _instructions.size()) {
        auto it = _instructions.begin();
        std::advance(it, index);
        return *it;
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

void BasicBlock::addInstructionAt(
    InstBase *inst, std::list<InstBase *>::iterator it
)
{
    _instructions.insert(it, inst);
}

void BasicBlock::addInstructionBefore(InstBase *inst, InstBase *before)
{
    auto it = std::find(_instructions.begin(), _instructions.end(), before);

    if (it != _instructions.end()) {
        _instructions.insert(it, inst);
    }
}

void BasicBlock::addInstructionAfter(InstBase *inst, InstBase *after)
{
    auto it = std::find(_instructions.begin(), _instructions.end(), after);

    if (it != _instructions.end()) {
        _instructions.insert(++it, inst);
    }
}

void BasicBlock::replaceInstruction(InstBase *oldInst, InstBase *newInst)
{
    auto it = std::find(_instructions.begin(), _instructions.end(), oldInst);

    if (it != _instructions.end()) {
        *it = newInst;
    }
}

void BasicBlock::removeInstruction(InstBase *inst)
{
    _instructions.remove(inst);
}

bool BasicBlock::containsInstruction(InstBase *inst) const
{
    return std::find(_instructions.begin(), _instructions.end(), inst)
        != _instructions.end();
}

void BasicBlock::setTerminator(InstBase *terminator)
{
    _instructions.push_back(terminator);
}

InstBase *BasicBlock::getTerminator() const
{
    return _instructions.back();
}

void BasicBlock::setLabel(std::string const &label)
{
    _label = label;
}

std::string BasicBlock::getLabel() const
{
    return _label;
}

} // namespace glu::gil
