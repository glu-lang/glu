#ifndef GLU_GIL_FUNCTION_HPP
#define GLU_GIL_FUNCTION_HPP

#include "BasicBlock.hpp"
#include "Types/Types.hpp"

namespace glu::gil {

/// @class Function
/// @brief Represents a function in the GIL (Glu Intermediate Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil
class Function {

public:
    using BBListType = llvm::iplist<BasicBlock>;

private:
    BBListType _basicBlocks;
    std::string _name;
    glu::types::FunctionTy *_type;

public:
    Function(std::string const &name, glu::types::FunctionTy *type)
        : _name(name), _type(type)
    {
    }
    ~Function();

    // defined to be used by ilist
    static BBListType Function::*getSublistAccess(BasicBlock *)
    {
        return &Function::_basicBlocks;
    }

    std::string const &getName() const { return _name; }
    void setName(std::string const &name) { _name = name; }

    glu::types::FunctionTy *getType() const { return _type; }

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

#endif // GLU_GIL_FUNCTION_HPP
