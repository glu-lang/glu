#include "IRGen.hpp"
#include "TypeLowering.hpp"

#include "GIL/InstVisitor.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

namespace glu::irgen {

/// @brief IRGenImpl is the implementation of the IRGen visitor.
/// It inherits from glu::gil::InstVisitor and provides the necessary methods
/// to visit different instruction types in the GIL intermediate representation.
/// This class is used to generate LLVM IR from GIL instructions.
struct IRGenVisitor : public glu::gil::InstVisitor<IRGenVisitor> {
    llvm::Module &outModule;
    llvm::LLVMContext &ctx;
    llvm::IRBuilder<> builder;

    // State
    llvm::Function *f = nullptr;
    llvm::BasicBlock *bb = nullptr;
    llvm::DenseMap<gil::Value, llvm::Value *> valueMap;

    IRGenVisitor(llvm::Module &module)
        : outModule(module), ctx(module.getContext()), builder(ctx)
    {
    }

    void beforeVisitFunction(glu::gil::Function *fn)
    {
        assert(!f && "Callbacks should be called in the right order");
        // Convert GIL function to LLVM function
        llvm::FunctionType *funcType
            = TypeLowering(ctx).visitFunctionTy(fn->getType());
        f = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, fn->getName(), outModule
        );
    }

    void afterVisitFunction([[maybe_unused]] glu::gil::Function *fn)
    {
        assert(f && "Callbacks should be called in the right order");
        // Verify the function (optional, good for debugging the compiler)
        llvm::verifyFunction(*f);
        f = nullptr;
    }

    void beforeVisitBasicBlock(glu::gil::BasicBlock *block)
    {
        assert(!bb && "Callbacks should be called in the right order");
        // Create a new LLVM basic block
        bb = llvm::BasicBlock::Create(ctx, block->getLabel(), f);
        // TODO: phi nodes for arguments
        builder.SetInsertPoint(bb);
    }

    void afterVisitBasicBlock([[maybe_unused]] glu::gil::BasicBlock *block)
    {
        assert(bb && "Callbacks should be called in the right order");
        bb = nullptr;
    }

    void visitUnreachableInst([[maybe_unused]] glu::gil::UnreachableInst *inst)
    {
        builder.CreateUnreachable();
    }

    llvm::Value *translateValue(gil::Value &value)
    {
        // Check if the value is already translated
        auto it = valueMap.find(value);
        if (it != valueMap.end()) {
            return it->second; // Return the existing LLVM value
        }

        // TODO: Is this possible?
        assert(false && "Value translation not implemented yet");
    }

    llvm::Type *translateType(gil::Type type)
    {
        return TypeLowering(ctx).visit(type.getType());
    }

    void visitReturnInst(glu::gil::ReturnInst *inst)
    {
        if (inst->getValue() == gil::Value::getEmptyKey()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(translateValue(inst->getValue()));
        }
    }

    void visitIntegerLiteralInst(glu::gil::IntegerLiteralInst *inst)
    {
        // Create an LLVM integer constant
        auto ty = llvm::cast<glu::types::IntTy>(inst->getType().getType());
        assert(
            ty->getBitWidth() == inst->getValue().getBitWidth()
            && "Integer literal type and value bit width mismatch"
        );
        llvm::Value *value = llvm::ConstantInt::get(ctx, inst->getValue());
        // Map the GIL value to the LLVM value
        valueMap[inst->getResult(0)] = value;
    }
};

void IRGen::generateIR(llvm::Module &out, glu::gil::Module *mod)
{
    IRGenVisitor visitor(out);
    // Visit the module to generate IR
    visitor.visit(mod);
}

} // namespace glu::irgen
