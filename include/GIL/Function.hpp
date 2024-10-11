#ifndef FUNCTION_HPP_
#define FUNCTION_HPP_

#include "BasicBlock.hpp"

namespace glu::gil {

class Function {

public:
    using BBListType = llvm::iplist<BasicBlock>;

private:
    BBListType _basicBlocks;

public:
    Function();
    ~Function();

    // defined to be used by ilist
    static BBListType Function::*getSublistAccess(BasicBlock *)
    {
        return &Function::_basicBlocks;
    }
};

} // end namespace glu::gil

#endif /* !FUNCTION_HPP_ */
