#include "PassManagerOptions.hpp"

namespace glu::optimizer {

// Define the command line options
llvm::cl::list<std::string> PassManagerOptions::_disablePasses(
    "disable-gil-pass", llvm::cl::desc("Disable specific GIL pass by name"),
    llvm::cl::ZeroOrMore, llvm::cl::value_desc("pass-name")
);

llvm::cl::list<std::string> PassManagerOptions::_printBeforePasses(
    "print-gil-before-pass", llvm::cl::desc("Print GIL before specific pass"),
    llvm::cl::ZeroOrMore, llvm::cl::value_desc("pass-name")
);

llvm::cl::list<std::string> PassManagerOptions::_printAfterPasses(
    "print-gil-after-pass", llvm::cl::desc("Print GIL after specific pass"),
    llvm::cl::ZeroOrMore, llvm::cl::value_desc("pass-name")
);

} // namespace glu::optimizer
