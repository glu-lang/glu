#ifndef GLU_GILGEN_PASSMANAGEROPTIONS_HPP
#define GLU_GILGEN_PASSMANAGEROPTIONS_HPP

#include "PassManager.hpp"
#include <llvm/Support/CommandLine.h>

namespace glu::optimizer {

/// @brief Command line options for the PassManager
class PassManagerOptions {
private:
    static llvm::cl::list<std::string> _disablePasses;
    static llvm::cl::list<std::string> _printBeforePasses;
    static llvm::cl::list<std::string> _printAfterPasses;

    static bool
    contains(llvm::StringRef passName, llvm::cl::list<std::string> const &list)
    {
        for (auto const &name : list) {
            if (name == passName)
                return true;
        }
        return false;
    }

public:
    static bool isDisabled(llvm::StringRef passName)
    {
        return contains(passName, _disablePasses);
    }

    static bool hasPrintBefore(llvm::StringRef passName)
    {
        return contains(passName, _printBeforePasses);
    }

    static bool hasPrintAfter(llvm::StringRef passName)
    {
        return contains(passName, _printAfterPasses);
    }
};

} // namespace glu::optimizer

#endif // GLU_GILGEN_PASSMANAGEROPTIONS_HPP
