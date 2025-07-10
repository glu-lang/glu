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

    // - MARK: Visitor Callbacks

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
        valueMap.clear();
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

    // - MARK: Value Translation

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

    // - MARK: Terminator Instructions

    void visitUnreachableInst([[maybe_unused]] glu::gil::UnreachableInst *inst)
    {
        builder.CreateUnreachable();
    }

    void visitReturnInst(glu::gil::ReturnInst *inst)
    {
        if (inst->getValue() == gil::Value::getEmptyKey()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(translateValue(inst->getValue()));
        }
    }

    // - MARK: Constant Instructions

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

    // - MARK: Memory Instructions

    void visitAllocaInst(glu::gil::AllocaInst *inst)
    {
        // Get the pointee type that we're allocating
        llvm::Type *pointeeType = translateType(inst->getPointeeType());

        // Save current insertion point
        auto savedIP = builder.saveIP();
        // Set insertion point to the start of the entry block
        llvm::BasicBlock &entry = f->getEntryBlock();
        builder.SetInsertPoint(&entry, entry.begin());
        // Create an alloca instruction at the start of the entry block
        llvm::Value *allocaValue = builder.CreateAlloca(pointeeType);
        // Restore previous insertion point
        builder.restoreIP(savedIP);
        mapValue(inst->getResult(0), allocaValue);
    }

    void visitLoadInst(glu::gil::LoadInst *inst)
    {
        // Get the pointer value to load from
        auto ptrValue = inst->getValue();
        llvm::Value *ptr = translateValue(ptrValue);

        // Get the type to load by looking at the result type
        llvm::Type *loadType = translateType(inst->getResultType(0));

        // Create a load instruction
        llvm::Value *loadedValue = builder.CreateLoad(loadType, ptr);
        mapValue(inst->getResult(0), loadedValue);
    }

    void visitStoreInst(glu::gil::StoreInst *inst)
    {
        // Get the source value and destination pointer
        auto sourceValue = inst->getSource();
        auto destValue = inst->getDest();
        llvm::Value *source = translateValue(sourceValue);
        llvm::Value *destPtr = translateValue(destValue);

        // Create a store instruction
        builder.CreateStore(source, destPtr);
        // StoreInst has no result to map
    }

    // - MARK: Call Instruction

    void visitCallInst(glu::gil::CallInst *inst)
    {
        // Prepare the arguments
        llvm::SmallVector<llvm::Value *, 8> args;
        args.reserve(inst->getArgs().size());
        for (auto arg : inst->getArgs()) {
            args.push_back(translateValue(arg));
        }

        llvm::CallInst *callInst;
        if (auto callee = inst->getFunctionOrNull()) {
            // Create a call to a named function
            callInst = builder.CreateCall(createOrGetFunction(callee), args);
        } else if (auto functionPtr = inst->getFunctionPtrValue()) {
            // Create a call to a function pointer
            auto ptrTy = llvm::dyn_cast<glu::types::PointerTy>(
                functionPtr->getType().getType()
            );
            assert(
                ptrTy && "Expected a pointer type for function pointer call"
            );
            auto funcTy
                = llvm::dyn_cast<glu::types::FunctionTy>(ptrTy->getPointee());
            assert(
                funcTy
                && "Expected a function type as pointee for function pointer"
            );
            callInst = builder.CreateCall(
                translateType(funcTy), translateValue(*functionPtr), args
            );
        } else {
            assert(
                false
                && "CallInst must have either a function or a function pointer"
            );
        }

        // Map the result if there is one
        if (inst->getResultCount() > 0) {
            mapValue(inst->getResult(0), callInst);
        }
    }

    // - MARK: Conversion Instructions

    void visitCastIntToPtrInst(glu::gil::CastIntToPtrInst *inst)
    {
        // Get the integer value to cast
        auto operand = inst->getOperand();
        llvm::Value *intValue = translateValue(operand);

        // Get the target pointer type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create an inttoptr instruction
        llvm::Value *ptrValue = builder.CreateIntToPtr(intValue, targetType);
        mapValue(inst->getResult(0), ptrValue);
    }

    void visitCastPtrToIntInst(glu::gil::CastPtrToIntInst *inst)
    {
        // Get the pointer value to cast
        auto operand = inst->getOperand();
        llvm::Value *ptrValue = translateValue(operand);

        // Get the target integer type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create a ptrtoint instruction
        llvm::Value *intValue = builder.CreatePtrToInt(ptrValue, targetType);
        mapValue(inst->getResult(0), intValue);
    }

    void visitBitcastInst(glu::gil::BitcastInst *inst)
    {
        // Get the value to bitcast
        auto operand = inst->getOperand();
        llvm::Value *value = translateValue(operand);

        // Get the target type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create a bitcast instruction
        llvm::Value *bitcastValue = builder.CreateBitCast(value, targetType);
        mapValue(inst->getResult(0), bitcastValue);
    }

    void visitIntTruncInst(glu::gil::IntTruncInst *inst)
    {
        // Get the integer value to truncate
        auto operand = inst->getOperand();
        llvm::Value *intValue = translateValue(operand);

        // Get the target integer type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create a trunc instruction
        llvm::Value *truncValue = builder.CreateTrunc(intValue, targetType);
        mapValue(inst->getResult(0), truncValue);
    }

    void visitIntZextInst(glu::gil::IntZextInst *inst)
    {
        // Get the integer value to zero-extend
        auto operand = inst->getOperand();
        llvm::Value *intValue = translateValue(operand);

        // Get the target integer type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create a zext instruction
        llvm::Value *zextValue = builder.CreateZExt(intValue, targetType);
        mapValue(inst->getResult(0), zextValue);
    }

    void visitIntSextInst(glu::gil::IntSextInst *inst)
    {
        // Get the integer value to sign-extend
        auto operand = inst->getOperand();
        llvm::Value *intValue = translateValue(operand);

        // Get the target integer type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create a sext instruction
        llvm::Value *sextValue = builder.CreateSExt(intValue, targetType);
        mapValue(inst->getResult(0), sextValue);
    }

    void visitFloatTruncInst(glu::gil::FloatTruncInst *inst)
    {
        // Get the float value to truncate
        auto operand = inst->getOperand();
        llvm::Value *floatValue = translateValue(operand);

        // Get the target float type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create an fptrunc instruction
        llvm::Value *truncValue = builder.CreateFPTrunc(floatValue, targetType);
        mapValue(inst->getResult(0), truncValue);
    }

    void visitFloatExtInst(glu::gil::FloatExtInst *inst)
    {
        // Get the float value to extend
        auto operand = inst->getOperand();
        llvm::Value *floatValue = translateValue(operand);

        // Get the target float type
        llvm::Type *targetType = translateType(inst->getDestType());

        // Create an fpext instruction
        llvm::Value *extValue = builder.CreateFPExt(floatValue, targetType);
        mapValue(inst->getResult(0), extValue);
    }
};

void IRGen::generateIR(llvm::Module &out, glu::gil::Module *mod)
{
    IRGenVisitor visitor(out);
    // Visit the module to generate IR
    visitor.visit(mod);
}

} // namespace glu::irgen
