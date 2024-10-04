#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include <llvm/ADT/ilist.h>
#include <llvm/ADT/ilist_node.h>
#include <string>

namespace glu::gil {
class BasicBlock;
class InstBase : public llvm::ilist_node_with_parent<InstBase, BasicBlock> {
    BasicBlock *parent = nullptr;
    friend class BasicBlock; // Allow BasicBlock to set itself as
public:
    InstBase() = default;
    virtual ~InstBase() = default;
    BasicBlock *getParent() const
    {
        return parent;
    }
    BasicBlock *getParent()
    {
        return parent;
    }
};

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
    BasicBlock(std::string label);
    ~BasicBlock() = default;

    llvm::ilist<InstBase> const &getInstructions() const;
    std::size_t getInstructionCount() const;
    InstBase *popFirstInstruction();
    void addInstructionAtEnd(InstBase *inst);
    void addInstructionAtStart(InstBase *inst);
    void addInstructionAt(InstBase *inst, llvm::ilist<InstBase>::iterator it);
    void addInstructionBefore(InstBase *inst, InstBase *before);
    void addInstructionAfter(InstBase *inst, InstBase *after);
    void replaceInstruction(InstBase *oldInst, InstBase *newInst);
    void removeInstruction(InstBase *inst);

    llvm::ilist<InstBase> &getSublistAccess(BasicBlock *Parent)
    {
        return _instructions;
    }

    void setTerminator(InstBase *terminator);
    InstBase *getTerminator() const;

    void setLabel(std::string label);
    std::string const &getLabel() const;

private:
    llvm::ilist<InstBase> _instructions;
    std::string _label;
};

} // namespace glu::gil

#endif // GLU_GIL_BASICBLOCK_HPP
