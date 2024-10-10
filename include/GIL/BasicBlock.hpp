#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include "InstBase.hpp"

#include <string>

namespace glu::gil {

/// @class BasicBlock
/// @brief Represents a basic block for instructions in the GIL (Glu Intermediate
/// Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil/#basic-blocks
class BasicBlock {

public:
    using InstListType = llvm::iplist<InstBase>;

private:
    InstListType _instructions;
    std::string _label;

public:
    BasicBlock(std::string label = "")
        : _label(label) {};
    ~BasicBlock() = default;

    InstListType const &getInstructions() const
    {
        return _instructions;
    }

    std::size_t getInstructionCount() const
    {
        return _instructions.size();
    }

    InstBase *popFirstInstruction();

    void addInstructionAtEnd(InstBase *inst)
    {
        _instructions.push_back(inst);
    }

    void addInstructionAtStart(InstBase *inst)
    {
        _instructions.push_front(inst);
    }

    void addInstructionAt(InstBase *inst, InstListType::iterator it)
    {
        _instructions.insert(it, inst);
    }

    void addInstructionBefore(InstBase *inst, InstBase *before);
    void addInstructionAfter(InstBase *inst, InstBase *after);
    void replaceInstruction(InstBase *oldInst, InstBase *newInst);
    void removeInstruction(InstBase *inst);

    // defined to be used by ilist
    static InstListType BasicBlock::*getSublistAccess(InstBase *)
    {
        return &BasicBlock::_instructions;
    }

    InstBase *getTerminator();
    TerminatorInst *getTerminatorInst();
    void setTerminator(InstBase *terminator);

    void setLabel(std::string label) { _label = label; }
    std::string const &getLabel() const { return _label; }
};

} // end namespace glu::gil

#endif // GLU_GIL_BASICBLOCK_HPP
