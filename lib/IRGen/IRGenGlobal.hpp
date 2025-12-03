#ifndef GLU_IRGEN_IRGENGLOBAL_HPP
#define GLU_IRGEN_IRGENGLOBAL_HPP

#include "Context.hpp"
#include "Mangling.hpp"
#include "TypeLowering.hpp"

#include "AST/Decls.hpp"
#include "GIL/Global.hpp"
#include "Types.hpp"
#include "Types/TypeVisitor.hpp"

#include <llvm/Transforms/Utils/ModuleUtils.h>

namespace glu::irgen {

class IRGenGlobal {
    Context &ctx;
    TypeLowering &typeLowering;

    // State
    llvm::DenseMap<glu::gil::Global *, llvm::GlobalVariable *>
        _globalStorageMap;
    llvm::DenseMap<glu::gil::Global *, llvm::Function *> _globalAccessorMap;

public:
    IRGenGlobal(Context &context, TypeLowering &typeLowering)
        : ctx(context), typeLowering(typeLowering)
    {
    }

private:
    // set-bit global variable to track initialization
    // only for non-const non-eager globals (default being lazy globals)
    llvm::GlobalVariable *createSetBit(gil::Global *g)
    {
        // Create the global variable
        auto *llvmType = llvm::Type::getInt1Ty(ctx.ctx);
        auto linkageName = mangleGlobalVariableSetBit(g->getDecl());
        auto *llvmGlobal = new llvm::GlobalVariable(
            ctx.outModule, llvmType, /* isConstant = */ false,
            llvm::GlobalValue::InternalLinkage,
            llvm::Constant::getNullValue(llvmType), linkageName
        );
        return llvmGlobal;
    }

    void generateLazyGlobal(
        gil::Global *g, llvm::Function *init, llvm::GlobalVariable *storage
    )
    {
        auto *accessor = getAccessor(g);
        auto *setBit = createSetBit(g);

        // Create the body of the accessor function
        // This function is called before every access to the global variable
        // It checks if the variable has been initialized, and if not, it calls
        // the initializer function
        llvm::IRBuilder<> builder(
            llvm::BasicBlock::Create(ctx.ctx, "entry", accessor)
        );
        auto *isSet
            = builder.CreateLoad(llvm::Type::getInt1Ty(ctx.ctx), setBit);
        auto *thenBB = llvm::BasicBlock::Create(ctx.ctx, "then", accessor);
        auto *elseBB = llvm::BasicBlock::Create(ctx.ctx, "else", accessor);
        builder.CreateCondBr(isSet, thenBB, elseBB);
        builder.SetInsertPoint(thenBB);
        builder.CreateRetVoid();
        builder.SetInsertPoint(elseBB);
        builder.CreateStore(llvm::ConstantInt::getTrue(ctx.ctx), setBit);
        auto *call = builder.CreateCall(init);
        builder.CreateStore(call, storage);
        builder.CreateRetVoid();
    }

    void generateEagerGlobal(
        gil::Global *g, llvm::Function *init, llvm::GlobalVariable *storage
    )
    {
        // Create a constructor function
        auto *funcType
            = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.ctx), false);
        auto linkageName
            = mangleGlobalVariableConstructorFunction(g->getDecl());
        auto *llvmFunction = llvm::Function::Create(
            funcType, llvm::Function::InternalLinkage, linkageName,
            ctx.outModule
        );
        // Create the body of the constructor function
        llvm::IRBuilder<> builder(
            llvm::BasicBlock::Create(ctx.ctx, "entry", llvmFunction)
        );
        auto *call = builder.CreateCall(init);
        builder.CreateStore(call, storage);
        builder.CreateRetVoid();
        // Register the constructor function to be called at program startup
        llvm::appendToGlobalCtors(
            ctx.outModule, llvmFunction, /* Priority = */ 65535, nullptr
        );
    }

public:
    // accessor function to initialize the global if needed
    // only for non-const non-eager globals (default being lazy globals)
    llvm::Function *getAccessor(gil::Global *g)
    {
        if (!g->hasInitializer()
            || g->getDecl()->hasAttribute(ast::AttributeKind::EagerKind)) {
            return nullptr; // No accessor needed
        }
        auto it = _globalAccessorMap.find(g);
        if (it != _globalAccessorMap.end()) {
            return it->second;
        }

        auto *funcType
            = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.ctx), false);
        auto linkageName = mangleGlobalVariableAccessorFunction(g->getDecl());
        auto *llvmFunction = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, linkageName,
            ctx.outModule
        );
        _globalAccessorMap.insert({ g, llvmFunction });
        return llvmFunction;
    }

    // storage global variable (the actual data, for all globals)
    llvm::GlobalVariable *getStorage(gil::Global *g)
    {
        auto it = _globalStorageMap.find(g);
        if (it != _globalStorageMap.end()) {
            return it->second;
        }

        // Create the global variable
        auto *llvmType = typeLowering.visit(g->getType());
        auto linkageName = mangleGlobalVariableStorage(g->getDecl());
        auto *llvmGlobal = new llvm::GlobalVariable(
            ctx.outModule, llvmType, /* isConstant = */ false,
            llvm::GlobalValue::ExternalLinkage, nullptr, linkageName
        );

        if (auto *structTy = llvm::dyn_cast<types::StructTy>(g->getType())) {
            if (structTy->getAlignment() > 0) {
                llvmGlobal->setAlignment(llvm::Align(structTy->getAlignment()));
            }
        }

        _globalStorageMap.insert({ g, llvmGlobal });
        return llvmGlobal;
    }

    void generateGlobal(gil::Global *g, llvm::Function *init)
    {
        auto *storage = getStorage(g);
        if (g->hasInitializer() && init == nullptr) {
            // Global has an external initializer
            storage->setExternallyInitialized(true);
            return;
        }
        storage->setInitializer(
            llvm::Constant::getNullValue(storage->getValueType())
        );
        if (init == nullptr) {
            // Global has no initializer, nothing to do
            return;
        }
        if (g->getDecl()->hasAttribute(ast::AttributeKind::EagerKind)) {
            // Eager global, init at program startup
            generateEagerGlobal(g, init, storage);
        } else {
            // Lazy global, init on first access
            generateLazyGlobal(g, init, storage);
        }
    }
};

} // namespace glu::irgen

#endif // GLU_IRGEN_IRGENGLOBAL_HPP
