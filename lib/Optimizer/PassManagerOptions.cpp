#include "PassManagerOptions.hpp"

namespace glu::optimizer::options {

// Define the command line options
llvm::cl::list<std::string> _disablePasses(
    "disable-gil-pass", llvm::cl::desc("Disable specific GIL pass by name"),
    llvm::cl::ZeroOrMore, llvm::cl::value_desc("pass-name")
);

llvm::cl::list<std::string> _printBeforePasses(
    "print-gil-before-pass", llvm::cl::desc("Print GIL before specific pass"),
    llvm::cl::ZeroOrMore, llvm::cl::value_desc("pass-name")
);

llvm::cl::list<std::string> _printAfterPasses(
    "print-gil-after-pass", llvm::cl::desc("Print GIL after specific pass"),
    llvm::cl::ZeroOrMore, llvm::cl::value_desc("pass-name")
);

llvm::cl::opt<bool> _printBeforeEachPasses(
    "print-gil-before-each-pass",
    llvm::cl::desc("Print GIL before each optimization pass"),
    llvm::cl::init(false)
);

llvm::cl::opt<bool> _printAfterEachPasses(
    "print-gil-after-each-pass",
    llvm::cl::desc("Print GIL after each optimization pass"),
    llvm::cl::init(false)
);

bool contains(llvm::StringRef passName, llvm::cl::list<std::string> const &list)
{
    for (auto const &name : list) {
        if (name == passName)
            return true;
    }
    return false;
}

bool isDisabled(llvm::StringRef passName)
{
    return contains(passName, _disablePasses);
}

bool hasPrintBefore(llvm::StringRef passName)
{
    return contains(passName, _printBeforePasses);
}

bool hasPrintAfter(llvm::StringRef passName)
{
    return contains(passName, _printAfterPasses);
}

bool hasPrintBeforeEachPasses()
{
    return _printBeforeEachPasses;
}

bool hasPrintAfterEachPasses()
{
    return _printAfterEachPasses;
}

} // namespace glu::optimizer
