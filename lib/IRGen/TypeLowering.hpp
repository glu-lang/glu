#ifndef GLU_IRGEN_TYPELOWERING_HPP
#define GLU_IRGEN_TYPELOWERING_HPP

#include "AST/Decls.hpp"
#include "GIL/Type.hpp"
#include "Types.hpp"
#include "Types/TypeVisitor.hpp"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace glu::irgen {

class TypeLowering
    : public glu::types::TypeVisitor<TypeLowering, llvm::Type *> {
    llvm::LLVMContext &ctx;
    llvm::DenseMap<types::StructTy *, llvm::StructType *> structMap;

public:
    TypeLowering(llvm::LLVMContext &context) : ctx(context) { }

    llvm::Type *visitTypeBase([[maybe_unused]] glu::types::TypeBase *type)
    {
        assert(false && "Unknown type kind");
        return nullptr;
    }

    llvm::Type *visitVoidTy([[maybe_unused]] glu::types::VoidTy *type)
    {
        return llvm::Type::getVoidTy(ctx);
    }

    llvm::Type *visitBoolTy([[maybe_unused]] glu::types::BoolTy *type)
    {
        return llvm::Type::getInt1Ty(ctx);
    }

    llvm::Type *visitCharTy([[maybe_unused]] glu::types::CharTy *type)
    {
        return llvm::Type::getInt8Ty(ctx);
    }

    llvm::Type *
    visitDynamicArrayTy([[maybe_unused]] glu::types::DynamicArrayTy *type)
    {
        return llvm::PointerType::get(ctx, 0); // opaque pointer
    }

    llvm::Type *visitEnumTy([[maybe_unused]] glu::types::EnumTy *type)
    {
        // Enums are represented as integers in LLVM
        return llvm::Type::getIntNTy(
            ctx, 32
        ); // TODO: Use the actual bit width of the enum
    }

    llvm::Type *visitIntTy(glu::types::IntTy *type)
    {
        return llvm::Type::getIntNTy(ctx, type->getBitWidth());
    }

    llvm::Type *visitFloatTy(glu::types::FloatTy *type)
    {
        switch (type->getBitWidth()) {
        case glu::types::FloatTy::HALF: return llvm::Type::getHalfTy(ctx);
        case glu::types::FloatTy::FLOAT: return llvm::Type::getFloatTy(ctx);
        case glu::types::FloatTy::DOUBLE: return llvm::Type::getDoubleTy(ctx);
        case glu::types::FloatTy::INTEL_LONG_DOUBLE:
            return llvm::Type::getX86_FP80Ty(ctx);
        default: assert(false && "Unknown float type"); return nullptr;
        }
    }

    llvm::FunctionType *visitFunctionTy(glu::types::FunctionTy *type)
    {
        llvm::SmallVector<llvm::Type *, 8> paramTypes;
        for (auto *param : type->getParameters()) {
            paramTypes.push_back(visit(param));
        }
        llvm::Type *returnType = visit(type->getReturnType());
        return llvm::FunctionType::get(returnType, paramTypes, false);
    }

    llvm::Type *visitPointerTy([[maybe_unused]] glu::types::PointerTy *type)
    {
        return llvm::PointerType::get(ctx, 0);
    }

    llvm::Type *visitStaticArrayTy(glu::types::StaticArrayTy *type)
    {
        return llvm::ArrayType::get(
            visit(type->getDataType()), type->getSize()
        );
    }

    llvm::Type *visitTypeAliasTy(glu::types::TypeAliasTy *type)
    {
        return visit(type->getWrappedType());
    }

    llvm::Type *visitStructTy(glu::types::StructTy *type)
    {
        if (auto it = structMap.find(type); it != structMap.end()) {
            return it->second;
        }

        llvm::SmallVector<llvm::Type *, 8> fieldTypes;
        for (auto &field : type->getFields()) {
            fieldTypes.push_back(visit(field->getType()));
        }
        auto *structType
            = llvm::StructType::create(ctx, fieldTypes, type->getName());
        structMap[type] = structType;
        return structType;
    }
};

} // namespace glu::irgen

#endif // GLU_IRGEN_TYPELOWERING_HPP
