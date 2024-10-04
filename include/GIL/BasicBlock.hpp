#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include <list>
#include <string>

class InstBase;

namespace glu::gil {

/**
 * @class BasicBlock
 * @brief Represents a basic block for instructions in the GIL (Glu Intermediate
 * Language).
 * See the documentation here for more information:
 * https://glu-lang.org/gil/#basic-blocks
 */
class BasicBlock {
public:
    BasicBlock();
    BasicBlock(
        std::string const &label, std::list<InstBase *> const &instructions
    );
    BasicBlock(std::list<InstBase *> const &instructions);
    ~BasicBlock() = default;

    std::list<InstBase *> getInstructions() const;
    std::size_t getInstructionCount() const;
    InstBase *popFirstInstruction();
    InstBase *getInstructionAt(std::size_t index) const;
    void addInstructionAtEnd(InstBase *inst);
    void addInstructionAtStart(InstBase *inst);
    void addInstructionAt(InstBase *inst, std::list<InstBase *>::iterator it);
    void addInstructionBefore(InstBase *inst, InstBase *before);
    void addInstructionAfter(InstBase *inst, InstBase *after);
    void replaceInstruction(InstBase *oldInst, InstBase *newInst);
    void removeInstruction(InstBase *inst);
    bool containsInstruction(InstBase *inst) const;

    void setTerminator(InstBase *terminator);
    InstBase *getTerminator() const;

    void setLabel(std::string const &label);
    std::string getLabel() const;

private:
    std::list<InstBase *> _instructions;
    std::string _label;
};

} // namespace glu::gil

#endif // GLU_GIL_BASICBLOCK_HPP
