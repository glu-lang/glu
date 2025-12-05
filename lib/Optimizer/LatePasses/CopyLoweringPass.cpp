#include "Basic/Diagnostic.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "PassManager.hpp"

namespace glu::optimizer {

/// @brief Basic CopyLoweringPass that replaces load [copy] instructions with
/// copy function calls. The copy function receives a pointer to the value.
class CopyLoweringPass : public gil::InstVisitor<CopyLoweringPass> {
private:
    gil::Module *module;
    std::optional<gilgen::Context> ctx = std::nullopt;
    std::vector<gil::InstBase *> _instructionsToRemove;
    glu::DiagnosticManager &_diagManager;
    gil::Function *_currentFunction = nullptr;

public:
    CopyLoweringPass(gil::Module *module, glu::DiagnosticManager &diagManager)
        : module(module), _diagManager(diagManager)
    {
    }

    void visitLoadInst(gil::LoadInst *loadInst)
    {
        // Only handle load [copy] instructions
        if (loadInst->getOwnershipKind() != gil::LoadOwnershipKind::Copy)
            return;

        if (!ctx)
            return;

        // Check if this is a struct type with an overloaded copy function
        auto *structure
            = llvm::dyn_cast<types::StructTy>(loadInst->getResultType());
        if (!structure || !structure->getDecl()->hasOverloadedCopyFunction()) {
            // Change the load to None ownership (no copy semantics)
            loadInst->setOwnershipKind(gil::LoadOwnershipKind::None);
            return;
        }

        // Check for infinite recursion: if we're inside the copy function
        // for this struct, warn about potential infinite recursion
        ast::FunctionDecl *copyFunc = structure->getDecl()->getCopyFunction();
        if (_currentFunction && _currentFunction->getDecl() == copyFunc) {
            _diagManager.warning(
                loadInst->getLocation(),
                "Copying '" + structure->getDecl()->getName().str()
                    + "' inside its own 'copy' overload will cause infinite "
                      "recursion"
            );
            _diagManager.note(
                copyFunc->getLocation(),
                "A struct passed by value to a function is implicitly copied. "
                "To avoid this, pass the struct by pointer or manually copy "
                "the fields"
            );
        }

        ctx->setInsertionPoint(loadInst->getParent(), loadInst);
        ctx->setSourceLoc(loadInst->getLocation());

        // Call the copy function with the original pointer
        auto *callInst = ctx->buildCall(
            structure->getDecl()->getCopyFunction(), { loadInst->getValue() }
        );

        loadInst->getResult(0).replaceAllUsesWith(callInst->getResult(0));
        // Mark the load instruction for removal
        _instructionsToRemove.push_back(loadInst);
    }

    void beforeVisitFunction(gil::Function *func)
    {
        // Create context for this function
        ctx.emplace(module, func);
        _currentFunction = func;
    }

    void afterVisitFunction(gil::Function *)
    {
        ctx.reset();
        _currentFunction = nullptr;
        for (auto *inst : _instructionsToRemove) {
            inst->eraseFromParent();
        }
        _instructionsToRemove.clear();
    }
};

void PassManager::runCopyLoweringPass()
{
    CopyLoweringPass pass(_module, _diagManager);
    pass.visit(_module);
}

} // namespace glu::optimizer
