#ifndef GLU_IRGEN_TYPELOWERING_HPP
#define GLU_IRGEN_TYPELOWERING_HPP

#include "Context.hpp"

#include "AST/Decls.hpp"
#include "AST/Exprs.hpp"
#include "Types/TypeVisitor.hpp"

#include <llvm/ADT/APSInt.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

#include <variant>

namespace glu::irgen {

class TypeLowering
    : public glu::types::TypeVisitor<TypeLowering, llvm::Type *> {
    llvm::LLVMContext &ctx;
    llvm::DenseMap<types::StructTy *, llvm::StructType *> structMap;

public:
    TypeLowering(llvm::LLVMContext &context) : ctx(context) { }

    llvm::Type *visitTypeBase([[maybe_unused]] glu::types::TypeBase *type)
    {
        llvm::errs() << "Unknown type kind: " << (int) type->getKind() << "\n";
        assert(false && "Unknown type kind");
        return nullptr;
    }

    llvm::Type *
    visitTemplateParamTy([[maybe_unused]] glu::types::TemplateParamTy *type)
    {
        llvm::errs() << "Unresolved template parameter type\n";
        assert(false && "Template parameter type should have been substituted");
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

    llvm::Type *visitEnumTy(glu::types::EnumTy *type)
    {
        if (auto *repr = type->getDecl()->getRepresentableType()) {
            return visit(repr);
        }

        return llvm::Type::getInt32Ty(ctx);
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

    llvm::Type *visitNullTy([[maybe_unused]] glu::types::NullTy *type)
    {
        return llvm::PointerType::get(ctx, 0);
    }

    llvm::StructType *visitStructTy(glu::types::StructTy *type)
    {
        if (auto it = structMap.find(type); it != structMap.end()) {
            return it->second;
        }

        llvm::SmallVector<llvm::Type *, 8> fieldTypes;
        for (size_t i = 0; i < type->getFieldCount(); ++i) {
            auto *fieldType = type->getSubstitutedFieldType(i);
            fieldTypes.push_back(visit(fieldType));
        }

        auto *structType = llvm::StructType::create(
            ctx, fieldTypes, type->getName(), type->isPacked()
        );
        structMap[type] = structType;
        return structType;
    }
};

class DebugTypeLowering
    : public glu::types::TypeVisitor<DebugTypeLowering, llvm::DIType *> {
    Context &ctx;
    TypeLowering &typeLowering;

public:
    DebugTypeLowering(Context &context, TypeLowering &typeLowering)
        : ctx(context), typeLowering(typeLowering)
    {
    }

    llvm::DIType *visitTypeBase([[maybe_unused]] glu::types::TypeBase *type)
    {
        assert(false && "Unknown type kind");
        return nullptr;
    }

    llvm::DIType *
    visitTemplateParamTy([[maybe_unused]] glu::types::TemplateParamTy *type)
    {
        assert(false && "Template parameter type should have been substituted");
        return nullptr;
    }

    llvm::DIType *visitVoidTy([[maybe_unused]] glu::types::VoidTy *type)
    {
        return nullptr;
    }

    llvm::DIType *visitBoolTy([[maybe_unused]] glu::types::BoolTy *type)
    {
        return ctx.dib.createBasicType("Bool", 8, llvm::dwarf::DW_ATE_boolean);
    }

    llvm::DIType *visitCharTy([[maybe_unused]] glu::types::CharTy *type)
    {
        return ctx.dib.createBasicType(
            "char", 8, llvm::dwarf::DW_ATE_signed_char
        );
    }

    llvm::DIType *
    visitDynamicArrayTy([[maybe_unused]] glu::types::DynamicArrayTy *type)
    {
        return ctx.dib.createPointerType(
            ctx.dib.createUnspecifiedType("DynamicArray"), 64
        );
    }

    llvm::DIType *visitEnumTy(glu::types::EnumTy *type)
    {
        // Enums are represented as integers in LLVM
        llvm::SmallVector<llvm::Metadata *, 8> cases;
        bool isUnsigned = false;
        if (auto *repr = type->getRepresentableType()) {
            while (auto *alias = llvm::dyn_cast<types::TypeAliasTy>(repr)) {
                repr = alias->getWrappedType();
            }
            if (auto *intTy = llvm::dyn_cast<types::IntTy>(repr)) {
                isUnsigned = intTy->isUnsigned();
            }
        }
        for (unsigned i = 0; i < type->getFieldCount(); ++i) {
            auto field = type->getField(i);
            auto *literal = llvm::dyn_cast<ast::LiteralExpr>(field->getValue());
            assert(literal && "Enum case value must be resolved by Sema");
            auto literalValue = literal->getValue();
            assert(
                std::holds_alternative<llvm::APInt>(literalValue)
                && "Enum case value must be an integer literal"
            );
            llvm::APSInt value(std::get<llvm::APInt>(literalValue), isUnsigned);
            cases.push_back(ctx.dib.createEnumerator(field->getName(), value));
        }

        auto *underlyingLLVM = typeLowering.visitEnumTy(type);
        auto *underlying = type->getRepresentableType()
            ? visit(type->getRepresentableType())
            : ctx.dib.createBasicType(
                  "Int", underlyingLLVM->getScalarSizeInBits(),
                  llvm::dwarf::DW_ATE_signed
              );

        return ctx.dib.createEnumerationType(
            ctx.getScopeForDecl(type->getDecl()), type->getName(),
            ctx.createDIFile(type->getLocation()),
            ctx.sm->getSpellingLineNumber(type->getLocation()),
            underlyingLLVM->getScalarSizeInBits(),
            underlyingLLVM->getScalarSizeInBits(),
            ctx.dib.getOrCreateArray(cases), underlying
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
        auto size = ctx.outModule.getDataLayout().getPointerSizeInBits();
        return ctx.dib.createPointerType(visit(type->getPointee()), size);
    }

    llvm::DIType *visitNullTy([[maybe_unused]] glu::types::NullTy *type)
    {
        auto size = ctx.outModule.getDataLayout().getPointerSizeInBits();
        return ctx.dib.createBasicType(
            "Null", size, llvm::dwarf::DW_ATE_unsigned
        );
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
        // Create the corresponding LLVM struct type to get layout info
        auto *llvmStructType = typeLowering.visitStructTy(type);

        // Get struct layout information
        auto *structLayout
            = ctx.outModule.getDataLayout().getStructLayout(llvmStructType);
        uint64_t structSizeInBits = structLayout->getSizeInBits();

        // Use custom alignment if specified, otherwise use computed alignment
        uint64_t structAlignInBits;
        if (type->getAlignment() > 0) {
            structAlignInBits = type->getAlignment() * 8;
        } else {
            structAlignInBits = structLayout->getAlignment().value() * 8;
        }

        llvm::SmallVector<llvm::Metadata *, 8> fieldTypes;
        for (unsigned i = 0; i < type->getFields().size(); ++i) {
            auto &field = type->getFields()[i];
            auto *substitutedFieldType = type->getSubstitutedFieldType(i);
            auto *fieldType = visit(substitutedFieldType);

            // Get field offset and size
            uint64_t fieldOffsetInBits
                = structLayout->getElementOffsetInBits(i);
            uint64_t fieldSizeInBits = fieldType->getSizeInBits();

            fieldTypes.push_back(ctx.dib.createMemberType(
                nullptr, field->getName(),
                ctx.createDIFile(field->getLocation()),
                ctx.sm->getSpellingLineNumber(field->getLocation()),
                fieldSizeInBits, 0, fieldOffsetInBits, llvm::DINode::FlagZero,
                fieldType
            ));
        }

        auto *structType = ctx.dib.createStructType(
            ctx.getScopeForDecl(type->getDecl()), type->getName(),
            ctx.createDIFile(type->getLocation()),
            ctx.sm->getSpellingLineNumber(type->getLocation()),
            structSizeInBits, structAlignInBits, llvm::DINode::FlagZero,
            nullptr, ctx.dib.getOrCreateArray(fieldTypes)
        );
        return structType;
    }
};

} // namespace glu::irgen

#endif // GLU_IRGEN_TYPELOWERING_HPP
