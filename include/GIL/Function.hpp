#ifndef FUNCTION_HPP_
#define FUNCTION_HPP_

#include "BasicBlock.hpp"

namespace glu::gil {

// TODO: add a return type and parameters
class Function {

public:
    using BBListType = llvm::iplist<BasicBlock>;

private:
    BBListType _basicBlocks;
    std::string _name;

public:
    Function(const std::string &name) : _name(name) { }
    ~Function();

    // defined to be used by ilist
    static BBListType Function::*getSublistAccess(BasicBlock *)
    {
        return &Function::_basicBlocks;
    }

    const std::string &getName() const { return _name; }
    void setName(const std::string &name) { _name = name; }

    BasicBlock *popFirstBlock();
    BBListType const &getBasicBlocks() const { return _basicBlocks; }
    BasicBlock *getEntryBlock() { return &_basicBlocks.front(); }
    std::size_t getBasicBlockCount() const { return _basicBlocks.size(); }

    void addBasicBlockBefore(BasicBlock *bb, BasicBlock *before);
    void addBasicBlockAfter(BasicBlock *bb, BasicBlock *before);
    void addBasicBlockAtEnd(BasicBlock *bb) { _basicBlocks.push_back(bb); }
    void addBasicBlockAtStart(BasicBlock *bb) { _basicBlocks.push_front(bb); }
    void addBasicBlockAt(BasicBlock *bb, BBListType::iterator it)
    {
        _basicBlocks.insert(it, bb);
    }

    void replaceBasicBlock(BasicBlock *oldBB, BasicBlock *newBB);

    void removeBasicBlock(BBListType::iterator it) { _basicBlocks.erase(it); }
    void removeBasicBlock(BasicBlock *bb) { _basicBlocks.remove(bb); }
};

} // end namespace glu::gil

#endif /* !FUNCTION_HPP_ */
