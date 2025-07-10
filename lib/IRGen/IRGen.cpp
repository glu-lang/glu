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
    TypeLowering typeLowering;

    // State
    llvm::Function *f = nullptr;
    llvm::BasicBlock *bb = nullptr;
    llvm::DenseMap<gil::Value, llvm::Value *> valueMap;

    IRGenVisitor(llvm::Module &module)
        : outModule(module)
        , ctx(module.getContext())
        , builder(ctx)
        , typeLowering(ctx)
    {
    }

    llvm::Function *createOrGetFunction(glu::gil::Function *fn)
    {
        // Check if the function already exists (as a forward declaration)
        llvm::Function *existingFunction = outModule.getFunction(fn->getName());
        if (existingFunction) {
            return existingFunction;
        }
        // Convert GIL function to LLVM function
        auto *funcType = translateType(fn->getType());
        return llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, fn->getName(), outModule
        );
    }

    void beforeVisitFunction(glu::gil::Function *fn)
    {
        assert(!f && "Callbacks should be called in the right order");

        f = createOrGetFunction(fn);

        // Set names for function arguments and map them to GIL values
        auto argCount = fn->getEntryBlock()->getArgumentCount();
        auto llvmArgIt = f->arg_begin();
        for (size_t i = 0; i < argCount; ++i, ++llvmArgIt) {
            // TODO: GIL Function should be able to have argument names
            valueMap[fn->getEntryBlock()->getArgument(i)] = &*llvmArgIt;
        }
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
        // TODO: phi nodes for arguments for non-entry blocks
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
        // Create an empty PHI as a placeholder
        return valueMap[value]
            = builder.CreatePHI(translateType(value.getType()), 0);
    }

    void mapValue(gil::Value value, llvm::Value *llvmValue)
    {
        // Map the GIL value to the LLVM value
        auto it = valueMap.find(value);
        if (it != valueMap.end()) {
            assert(
                llvm::isa<llvm::PHINode>(it->second)
                && "Existing value must be an empty PHI node temporary"
            );
            it->second->replaceAllUsesWith(llvmValue); // Replace existing uses
            llvm::cast<llvm::PHINode>(it->second)
                ->eraseFromParent(); // Remove old value
            it->second = llvmValue; // Update existing mapping
        } else {
            valueMap[value] = llvmValue; // Create new mapping
        }
    }

    llvm::Type *translateType(gil::Type type)
    {
        return typeLowering.visit(type.getType());
    }

    llvm::FunctionType *translateType(types::FunctionTy *type)
    {
        return typeLowering.visitFunctionTy(type);
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
        mapValue(inst->getResult(0), value);
    }

    void visitFloatLiteralInst(glu::gil::FloatLiteralInst *inst)
    {
        // Create an LLVM floating point constant
        auto ty = llvm::cast<glu::types::FloatTy>(inst->getType().getType());
        llvm::Type *llvmType = typeLowering.visitFloatTy(ty);
        llvm::Value *value = llvm::ConstantFP::get(llvmType, inst->getValue());
        mapValue(inst->getResult(0), value);
    }

    void visitStringLiteralInst(glu::gil::StringLiteralInst *inst)
    {
        // Create a global string constant
        llvm::Value *value = builder.CreateGlobalString(inst->getValue());
        mapValue(inst->getResult(0), value);
    }

    void visitFunctionPtrInst(glu::gil::FunctionPtrInst *inst)
    {
        // Get the function from the module by name
        llvm::Function *llvmFunction = createOrGetFunction(inst->getFunction());

        mapValue(inst->getResult(0), llvmFunction);
    }

    void visitEnumVariantInst(glu::gil::EnumVariantInst *inst)
    {
        // Enum variants are represented as integer constants
        auto member = inst->getMember();
        auto enumTy
            = llvm::cast<glu::types::EnumTy>(member.getParent().getType());

        // Get the variant index by name
        auto variantIndexOpt = enumTy->getCaseIndex(member.getName());
        assert(variantIndexOpt.has_value() && "Enum variant not found");
        uint32_t variantIndex = static_cast<uint32_t>(variantIndexOpt.value());

        llvm::Type *enumLLVMTy = typeLowering.visitEnumTy(enumTy);
        llvm::Value *value = llvm::ConstantInt::get(enumLLVMTy, variantIndex);
        mapValue(inst->getResult(0), value);
    }
};

void IRGen::generateIR(llvm::Module &out, glu::gil::Module *mod)
{
    IRGenVisitor visitor(out);
    // Visit the module to generate IR
    visitor.visit(mod);
}

} // namespace glu::irgen
