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

    // Template implementation that can handle different method signatures
    // Template specialization for CreateZExt which has an extra parameter
    template <auto MethodPtr>
    void processConversionInstT(glu::gil::ConversionInst *inst)
    {
        auto operand = inst->getOperand();
        llvm::Value *srcValue = translateValue(operand);
        llvm::Type *targetType = translateType(inst->getDestType());
        llvm::Value *result = (builder.*MethodPtr)(srcValue, targetType, "");
        mapValue(inst->getResult(0), result);
    }

    // Specialization for CreateZExt which has a different signature
    template <>
    void processConversionInstT<&llvm::IRBuilderBase::CreateZExt>(
        glu::gil::ConversionInst *inst
    )
    {
        auto operand = inst->getOperand();
        llvm::Value *srcValue = translateValue(operand);
        llvm::Type *targetType = translateType(inst->getDestType());
        llvm::Value *result
            = builder.CreateZExt(srcValue, targetType, "", false);
        mapValue(inst->getResult(0), result);
    }

    // Macro to define visit methods for conversion instructions using the
    // template
#define DEFINE_CONVERSION_VISIT(InstClass, BuilderMethod)                  \
    void visit##InstClass(glu::gil::InstClass *inst)                       \
    {                                                                      \
        processConversionInstT<&llvm::IRBuilderBase::BuilderMethod>(inst); \
    }

    DEFINE_CONVERSION_VISIT(CastIntToPtrInst, CreateIntToPtr)
    DEFINE_CONVERSION_VISIT(CastPtrToIntInst, CreatePtrToInt)
    DEFINE_CONVERSION_VISIT(BitcastInst, CreateBitCast)
    DEFINE_CONVERSION_VISIT(IntTruncInst, CreateTrunc)
    DEFINE_CONVERSION_VISIT(IntSextInst, CreateSExt)
    DEFINE_CONVERSION_VISIT(IntZextInst, CreateZExt)
    DEFINE_CONVERSION_VISIT(FloatTruncInst, CreateFPTrunc)
    DEFINE_CONVERSION_VISIT(FloatExtInst, CreateFPExt)

#undef DEFINE_CONVERSION_VISIT

    // - MARK: Aggregate Instructions

    void visitStructExtractInst(glu::gil::StructExtractInst *inst)
    {
        auto structValue = inst->getStructValue();
        llvm::Value *structVal = translateValue(structValue);
        auto member = inst->getMember();

        auto structTy
            = llvm::cast<glu::types::StructTy>(structValue.getType().getType());
        auto fieldIndexOpt = structTy->getFieldIndex(member.getName());
        assert(fieldIndexOpt.has_value() && "Field not found in struct");
        uint32_t fieldIndex = static_cast<uint32_t>(fieldIndexOpt.value());

        llvm::Value *result = builder.CreateExtractValue(structVal, fieldIndex);
        mapValue(inst->getResult(0), result);
    }

    void visitStructCreateInst(glu::gil::StructCreateInst *inst)
    {
        llvm::Type *structType = translateType(inst->getStruct());
        llvm::Value *structVal = llvm::UndefValue::get(structType);

        auto fieldValues = inst->getMembers();
        for (size_t i = 0; i < fieldValues.size(); ++i) {
            gil::Value fieldValue = fieldValues[i]; // Copy to non-const
            llvm::Value *fieldVal = translateValue(fieldValue);
            structVal = builder.CreateInsertValue(
                structVal, fieldVal, static_cast<uint32_t>(i)
            );
        }

        mapValue(inst->getResult(0), structVal);
    }

    void visitStructDestructureInst(glu::gil::StructDestructureInst *inst)
    {
        auto structValue = inst->getStructValue();
        llvm::Value *structVal = translateValue(structValue);

        auto structTy
            = llvm::cast<glu::types::StructTy>(structValue.getType().getType());
        size_t fieldCount = structTy->getFieldCount();

        // Extract each field and map to results
        for (size_t i = 0; i < fieldCount; ++i) {
            llvm::Value *fieldVal = builder.CreateExtractValue(
                structVal, static_cast<uint32_t>(i)
            );
            mapValue(inst->getResult(i), fieldVal);
        }
    }

    void visitStructFieldPtrInst(glu::gil::StructFieldPtrInst *inst)
    {
        auto structValue = inst->getStructValue();
        llvm::Value *structPtr = translateValue(structValue);
        auto member = inst->getMember();

        auto structTy
            = llvm::cast<glu::types::StructTy>(member.getParent().getType());
        auto fieldIndexOpt = structTy->getFieldIndex(member.getName());
        assert(fieldIndexOpt.has_value() && "Field not found in struct");
        uint32_t fieldIndex = static_cast<uint32_t>(fieldIndexOpt.value());

        // Create GEP instruction to get field pointer
        // GEP with indices [0, fieldIndex] for struct field access
        llvm::Value *indices[] = {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0),
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), fieldIndex)
        };

        llvm::Value *fieldPtr = builder.CreateGEP(
            translateType(member.getParent()), structPtr, indices
        );

        mapValue(inst->getResult(0), fieldPtr);
    }

    void visitPtrOffsetInst(glu::gil::PtrOffsetInst *inst)
    {
        auto basePtr = inst->getBasePointer();
        auto offset = inst->getOffset();
        llvm::Value *basePtrVal = translateValue(basePtr);
        llvm::Value *offsetVal = translateValue(offset);

        auto ptrTy
            = llvm::cast<glu::types::PointerTy>(basePtr.getType().getType());
        llvm::Type *pointeeType = typeLowering.visit(ptrTy->getPointee());

        llvm::Value *result
            = builder.CreateGEP(pointeeType, basePtrVal, offsetVal);

        mapValue(inst->getResult(0), result);
    }
};

void IRGen::generateIR(llvm::Module &out, glu::gil::Module *mod)
{
    IRGenVisitor visitor(out);
    // Visit the module to generate IR
    visitor.visit(mod);
}

} // namespace glu::irgen
