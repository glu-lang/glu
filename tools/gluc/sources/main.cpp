#include "GIL/BasicBlock.hpp"
#include <iostream>

int main()
{
    glu::gil::BasicBlock bb;
    glu::gil::InstBase *inst_ptr = new glu::gil::InstBase;
    glu::gil::InstBase *instB_ptr = new glu::gil::InstBase;

    bb.addInstructionAtEnd(inst_ptr);
    std::cout << "Instruction count: " << bb.getInstructionCount() << std::endl;
    std::cout << "Parent of inst is: "
              << static_cast<void *>(inst_ptr->getParent()) << std::endl;

    bb.addInstructionBefore(instB_ptr, inst_ptr);
    std::cout << "Instruction count: " << bb.getInstructionCount() << std::endl;
    std::cout << "Parent of instB is: "
              << static_cast<void *>(instB_ptr->getParent()) << std::endl;

    glu::gil::InstBase *test = bb.popFirstInstruction();
    std::cout << "Instruction count: " << bb.getInstructionCount() << std::endl;
    std::cout << "Parent of test is: " << static_cast<void *>(test->getParent())
              << std::endl;
    delete test;

    auto terminator = bb.getTerminatorInst();
    std::cout << "Terminator: " << static_cast<void *>(terminator) << std::endl;
    return 0;
}
