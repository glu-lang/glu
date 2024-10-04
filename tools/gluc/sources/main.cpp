#include "GIL/BasicBlock.hpp"
#include <iostream>

int main()
{
    glu::gil::BasicBlock bb;
    glu::gil::InstBase inst;
    glu::gil::InstBase instB;

    bb.addInstructionAtEnd(&inst);
    std::cout << "Instruction count: " << bb.getInstructionCount() << std::endl;
    std::cout << "Parent of inst is: " << static_cast<void *>(inst.getParent())
              << std::endl;

    bb.addInstructionBefore(&instB, &inst);
    std::cout << "Instruction count: " << bb.getInstructionCount() << std::endl;
    std::cout << "Parent of instB is: "
              << static_cast<void *>(instB.getParent()) << std::endl;

    auto test = bb.popFirstInstruction();
    std::cout << "Instruction count: " << bb.getInstructionCount() << std::endl;
    std::cout << "Parent of test is: " << static_cast<void *>(test->getParent())
              << std::endl;
    return 0;
}
