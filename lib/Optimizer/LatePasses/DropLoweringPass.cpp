#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "PassManager.hpp"

namespace glu::optimizer {

class DropLoweringPass : public gil::InstVisitor<DropLoweringPass> {
private:
    gil::Module *module;
    std::optional<gilgen::Context> ctx = std::nullopt;
    llvm::SmallVector<gil::InstBase *, 8> toErase;

public:
    DropLoweringPass(gil::Module *module) : module(module) { }

    ~DropLoweringPass()
    {
        for (auto *inst : toErase) {
            inst->eraseFromParent();
        }
    }

    void visitDropInst(gil::DropInst *dropInst)
    {
        if (!ctx)
            return;

        auto *bb = dropInst->getParent();
        ctx->setInsertionPoint(bb, dropInst);
        ctx->setSourceLoc(dropInst->getLocation());

        // Generate code to call the drop function if it exists
        if (auto *structure
            = llvm::dyn_cast<types::StructTy>(dropInst->getValue().getType())) {
            if (structure->getDecl()->hasOverloadedDropFunction()) {
                // Get the pointer type from the struct's module context
                auto *ptrType = structure->getDecl()
                                    ->getModule()
                                    ->getContext()
                                    ->getTypesMemoryArena()
                                    .create<types::PointerTy>(structure);
                // Allocate a temporary to store the value
                auto *alloca = new gil::AllocaInst(structure, ptrType);
                alloca->setLocation(dropInst->getLocation());
                bb->addInstructionBefore(alloca, dropInst);
                auto ptr = alloca->getResult(0);
                // Store the value into the temporary
                ctx->buildStore(dropInst->getValue(), ptr);
                // Call the drop function with the pointer
                ctx->buildCall(
                    structure->getDecl()->getDropFunction(), { ptr }
                );
            }
        }

        // Remove the original drop instruction
        toErase.push_back(dropInst);
    }

    void beforeVisitFunction(gil::Function *func)
    {
        // Create context for this function
        ctx.emplace(module, func);
    }

    void afterVisitFunction(gil::Function *) { ctx.reset(); }
};

void PassManager::runDropLoweringPass()
{
    DropLoweringPass pass(_module);
    pass.visit(_module);
}

} // namespace glu::optimizer
