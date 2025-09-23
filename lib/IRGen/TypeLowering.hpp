#ifndef GLU_IRGEN_TYPELOWERING_HPP
#define GLU_IRGEN_TYPELOWERING_HPP

#include "Context.hpp"

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
        return llvm::FunctionType::get(
            returnType, paramTypes, type->isCVariadic()
        );
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

class DebugTypeLowering
    : public glu::types::TypeVisitor<DebugTypeLowering, llvm::DIType *> {
    Context &ctx;

public:
    DebugTypeLowering(Context &context) : ctx(context) { }

    llvm::DIType *visitTypeBase([[maybe_unused]] glu::types::TypeBase *type)
    {
        assert(false && "Unknown type kind");
        return nullptr;
    }

    llvm::DIType *visitVoidTy([[maybe_unused]] glu::types::VoidTy *type)
    {
        return nullptr;
    }

    llvm::DIType *visitBoolTy([[maybe_unused]] glu::types::BoolTy *type)
    {
        return ctx.dib.createBasicType("Bool", 1, llvm::dwarf::DW_ATE_boolean);
    }

    llvm::DIType *visitCharTy([[maybe_unused]] glu::types::CharTy *type)
    {
        return ctx.dib.createBasicType(
            "Char", 8, llvm::dwarf::DW_ATE_signed_char
        );
    }

    llvm::DIType *
    visitDynamicArrayTy([[maybe_unused]] glu::types::DynamicArrayTy *type)
    {
        return ctx.dib.createPointerType(
            ctx.dib.createUnspecifiedType("DynamicArray"), 64
        );
    }

    llvm::DIType *visitEnumTy([[maybe_unused]] glu::types::EnumTy *type)
    {
        // Enums are represented as integers in LLVM
        llvm::SmallVector<llvm::Metadata *, 8> cases;
        for (unsigned i = 0; i < type->getFieldCount(); ++i) {
            auto field = type->getField(i);
            cases.push_back(ctx.dib.createEnumerator(field->getName(), i));
        }
        return ctx.dib.createEnumerationType(
            nullptr, type->getName(), ctx.createDIFile(type->getLocation()),
            ctx.sm->getSpellingLineNumber(type->getLocation()), 32, 32,
            ctx.dib.getOrCreateArray(cases),
            ctx.dib.createBasicType("Int", 32, llvm::dwarf::DW_ATE_signed)
        );
    }

    llvm::DIType *visitIntTy(glu::types::IntTy *type)
    {
        return ctx.dib.createBasicType(
            "Int", type->getBitWidth(),
            type->getSignedness() == types::IntTy::Signed
                ? llvm::dwarf::DW_ATE_signed
                : llvm::dwarf::DW_ATE_unsigned
        );
    }

    llvm::DIType *visitFloatTy(glu::types::FloatTy *type)
    {

        switch (type->getBitWidth()) {
        case glu::types::FloatTy::HALF:
            return ctx.dib.createBasicType(
                "Float16", 16, llvm::dwarf::DW_ATE_float
            );
        case glu::types::FloatTy::FLOAT:
            return ctx.dib.createBasicType(
                "Float", 32, llvm::dwarf::DW_ATE_float
            );
        case glu::types::FloatTy::DOUBLE:
            return ctx.dib.createBasicType(
                "Double", 64, llvm::dwarf::DW_ATE_float
            );
        case glu::types::FloatTy::INTEL_LONG_DOUBLE:
            return ctx.dib.createBasicType(
                "Float80", 80, llvm::dwarf::DW_ATE_float
            );
        default: assert(false && "Unknown float type"); return nullptr;
        }
    }

    llvm::DISubroutineType *visitFunctionTy(glu::types::FunctionTy *type)
    {
        llvm::SmallVector<llvm::Metadata *, 8> paramTypes;
        paramTypes.push_back(visit(type->getReturnType()));
        for (auto *param : type->getParameters()) {
            paramTypes.push_back(visit(param));
        }
        return ctx.dib.createSubroutineType(
            ctx.dib.getOrCreateTypeArray(paramTypes)
        );
    }

    llvm::DIType *visitPointerTy([[maybe_unused]] glu::types::PointerTy *type)
    {
        return ctx.dib.createPointerType(visit(type->getPointee()), 64);
    }

    llvm::DIType *visitStaticArrayTy(glu::types::StaticArrayTy *type)
    {
        return ctx.dib.createArrayType(
            0, 0, visit(type->getDataType()),
            ctx.dib.getOrCreateArray(
                { ctx.dib.getOrCreateSubrange(0, type->getSize()) }
            )
        );
    }

    llvm::DIType *visitTypeAliasTy(glu::types::TypeAliasTy *type)
    {
        return ctx.dib.createTypedef(
            visit(type->getWrappedType()), type->getName(),
            ctx.createDIFile(type->getLocation()),
            ctx.sm->getSpellingLineNumber(type->getLocation()), nullptr
        );
    }

    llvm::DIType *visitStructTy(glu::types::StructTy *type)
    {
        llvm::SmallVector<llvm::Metadata *, 8> fieldTypes;
        for (auto &field : type->getFields()) {
            fieldTypes.push_back(ctx.dib.createMemberType(
                nullptr, field->getName(),
                ctx.createDIFile(field->getLocation()),
                ctx.sm->getSpellingLineNumber(field->getLocation()), 0, 0, 0,
                llvm::DINode::FlagZero, visit(field->getType())
            ));
        }
        auto *structType = ctx.dib.createStructType(
            nullptr, type->getName(), ctx.createDIFile(type->getLocation()),
            ctx.sm->getSpellingLineNumber(type->getLocation()), 0, 0,
            llvm::DINode::FlagZero, nullptr,
            ctx.dib.getOrCreateArray(fieldTypes)
        );
        return structType;
    }
};

} // namespace glu::irgen

#endif // GLU_IRGEN_TYPELOWERING_HPP
