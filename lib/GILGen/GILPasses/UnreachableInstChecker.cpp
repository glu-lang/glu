#include "Basic/Diagnostic.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/UnreachableInst.hpp"
#include "PassManager.hpp"

namespace glu::gilgen {

/// @brief GIL Pass that detects unreachable instructions in reachable blocks.
///
/// This pass runs AFTER Dead Code Elimination (DCE), which removes all
/// unreachable basic blocks. Therefore, any UnreachableInst found indicates
/// a function that doesn't return on all code paths.
///
/// This is more accurate than AST-level checking, as it correctly handles
/// if-else branches and other control flow patterns.
class UnreachableInstChecker : public gil::InstVisitor<UnreachableInstChecker> {
private:
    DiagnosticManager &diagManager;

public:
    UnreachableInstChecker(DiagnosticManager &diagManager)
        : diagManager(diagManager)
    {
    }

    void visitUnreachableInst(gil::UnreachableInst *inst)
    {
        // After DCE, all basic blocks are reachable.
        // Any UnreachableInst means the function doesn't return on all paths.
        auto *bb = inst->getParent();
        if (!bb)
            return;

        auto *func = bb->getParent();
        if (!func)
            return;

        auto *decl = func->getDecl();
        if (!decl)
            return;

        diagManager.error(
            decl->getLocation(),
            llvm::Twine("Function '") + decl->getName().str()
                + "' does not end with a return statement"
        );
    }
};

void PassManager::runUnreachableInstChecker(
    gil::Module *module, llvm::BumpPtrAllocator & /* arena */,
    DiagnosticManager &diagManager
)
{
    UnreachableInstChecker pass(diagManager);
    pass.visit(module);
}

} // namespace glu::gilgen
