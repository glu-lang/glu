#include "TypeConverter.hpp"

#include "AST/Attributes.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include "Basic/MemoryArena.hpp"
#include "Basic/SourceLocation.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Casting.h>

namespace glu::clangimporter {

glu::types::TypeBase *TypeConverter::convert(clang::QualType clangType)
{
    // Check cache first
    clang::Type const *canonicalType
        = clangType.getCanonicalType().getTypePtr();
    if (auto type = _ctx.typeCache.lookup(canonicalType)) {
        return type;
    }

    auto &typesArena = _ctx.glu.getTypesMemoryArena();
    glu::types::TypeBase *gluType = nullptr;

    if (auto *builtinType = llvm::dyn_cast<clang::BuiltinType>(canonicalType)) {
        gluType = convertBuiltinType(builtinType);
    } else if (canonicalType->isPointerType()) {
        auto pointeeType
            = convert(canonicalType->getPointeeType().getCanonicalType());
        if (!pointeeType) {
            // Default to void pointer if pointee type is unknown
            pointeeType = typesArena.create<types::VoidTy>();
        }
        gluType = typesArena.create<types::PointerTy>(pointeeType);
    } else if (auto *recordType
               = llvm::dyn_cast<clang::RecordType>(canonicalType)) {
        gluType = convertRecordType(recordType, canonicalType);
    } else if (auto *enumType
               = llvm::dyn_cast<clang::EnumType>(canonicalType)) {
        gluType = convertEnumType(enumType, canonicalType);
    } else if (auto *arrayType
               = llvm::dyn_cast<clang::ConstantArrayType>(canonicalType)) {
        auto elementType = convert(arrayType->getElementType());
        if (!elementType) {
            return nullptr;
        }
        uint64_t size = arrayType->getSize().getZExtValue();
        gluType = typesArena.create<types::StaticArrayTy>(elementType, size);
    } else if (auto *funcProto
               = llvm::dyn_cast<clang::FunctionProtoType>(canonicalType)) {
        gluType = convertFunctionType(funcProto);
    } else {
        // Unsupported type - nullptr
        // Structs/enums are handled during declaration import and added to the
        // cache there
    }

    if (gluType) {
        _ctx.typeCache[canonicalType] = gluType;
    }
    return gluType;
}

glu::types::TypeBase *TypeConverter::convertRecordType(
    clang::RecordType const *type, clang::Type const *canonicalType
)
{
    auto *recordDecl = type->getDecl();
    if (auto *definition = recordDecl->getDefinition()) {
        recordDecl = definition;
    }

    // Skip anonymous structs for now
    if (!recordDecl->getIdentifier()) {
        return nullptr;
    }

    if (auto cached = _ctx.typeCache.lookup(canonicalType)) {
        return cached;
    }

    auto &astArena = _ctx.glu.getASTMemoryArena();
    auto &allocator = astArena.getAllocator();
    auto &typesArena = _ctx.glu.getTypesMemoryArena();

    llvm::SmallVector<glu::ast::FieldDecl *, 16> fields;
    auto *placeholderType = typesArena.create<types::VoidTy>();

    if (recordDecl->isCompleteDefinition()) {
        unsigned fieldIndex = 0;
        for (auto *field : recordDecl->fields()) {
            llvm::StringRef fieldName = field->getName();
            if (fieldName.empty()) {
                fieldName = copyString(
                    ("field" + llvm::Twine(fieldIndex)).str(), allocator
                );
            } else {
                fieldName = copyString(fieldName, allocator);
            }

            auto *fieldDecl = astArena.create<glu::ast::FieldDecl>(
                SourceLocation::invalid, fieldName, placeholderType, nullptr,
                nullptr, glu::ast::Visibility::Public
            );
            fields.push_back(fieldDecl);
            fieldIndex++;
        }
    }

    llvm::StringRef structName = copyString(recordDecl->getName(), allocator);
    auto *structDecl = glu::ast::StructDecl::create(
        allocator, _ctx.glu, SourceLocation::invalid, nullptr, structName,
        fields, nullptr, glu::ast::Visibility::Public, nullptr
    );

    auto *structType = structDecl->getType();
    _ctx.typeCache[canonicalType] = structType;
    _ctx.importedDecls.push_back(structDecl);

    if (recordDecl->isCompleteDefinition()) {
        unsigned fieldIndex = 0;
        for (auto *field : recordDecl->fields()) {
            auto fieldType = convert(field->getType());
            if (!fieldType) {
                fieldType = placeholderType;
            }
            fields[fieldIndex]->setType(fieldType);
            fieldIndex++;
        }
    }

    return structType;
}

glu::types::TypeBase *TypeConverter::convertEnumType(
    clang::EnumType const *type, clang::Type const *canonicalType
)
{
    auto *enumDecl = type->getDecl();
    if (auto *definition = enumDecl->getDefinition()) {
        enumDecl = definition;
    }

    // Skip anonymous enums for now
    if (!enumDecl->getIdentifier()) {
        return nullptr;
    }

    if (auto cached = _ctx.typeCache.lookup(canonicalType)) {
        return cached;
    }

    auto &astArena = _ctx.glu.getASTMemoryArena();
    auto &allocator = astArena.getAllocator();

    llvm::SmallVector<glu::ast::FieldDecl *, 16> cases;
    if (enumDecl->isCompleteDefinition()) {
        for (auto *enumConst : enumDecl->enumerators()) {
            llvm::StringRef caseName
                = copyString(enumConst->getName(), allocator);

            auto *caseDecl = astArena.create<glu::ast::FieldDecl>(
                SourceLocation::invalid, caseName, nullptr, nullptr, nullptr,
                glu::ast::Visibility::Public
            );
            cases.push_back(caseDecl);
        }
    }

    auto underlyingType = enumDecl->isCompleteDefinition()
        ? convert(enumDecl->getIntegerType())
        : nullptr;

    llvm::StringRef enumName = copyString(enumDecl->getName(), allocator);
    auto *gluEnumDecl = glu::ast::EnumDecl::create(
        allocator, _ctx.glu, SourceLocation::invalid, nullptr, enumName, cases,
        underlyingType, glu::ast::Visibility::Public, nullptr
    );

    auto *enumType = gluEnumDecl->getType();
    _ctx.typeCache[canonicalType] = enumType;
    _ctx.importedDecls.push_back(gluEnumDecl);

    return enumType;
}

glu::types::TypeBase *
TypeConverter::convertBuiltinType(clang::BuiltinType const *type)
{
    auto &typesArena = _ctx.glu.getTypesMemoryArena();
    auto bitWidth = _ctx.clang->getTypeInfo(type).Width;

    // "char" type (without signed or unsigned keywords)
    // is either signed or unsigned depending on the target, which is why we
    // check for both first.
    if (type->getKind() == clang::BuiltinType::Char_S
        || type->getKind() == clang::BuiltinType::Char_U) {
        return typesArena.create<types::CharTy>();
    }

    if (type->isSignedInteger()) {
        return typesArena.create<types::IntTy>(
            types::IntTy::Signedness::Signed, bitWidth
        );
    }

    if (type->isUnsignedInteger()) {
        return typesArena.create<types::IntTy>(
            types::IntTy::Signedness::Unsigned, bitWidth
        );
    }

    if (type->isFloatingPoint()) {
        return typesArena.create<types::FloatTy>(bitWidth);
    }

    if (type->isVoidType()) {
        return typesArena.create<types::VoidTy>();
    }
    if (type->isBooleanType()) {
        return typesArena.create<types::BoolTy>();
    }

    return nullptr;
}

glu::types::TypeBase *
TypeConverter::convertFunctionType(clang::FunctionProtoType const *funcType)
{
    auto &typesArena = _ctx.glu.getTypesMemoryArena();

    std::vector<glu::types::TypeBase *> paramTypes;
    for (auto paramType : funcType->getParamTypes()) {
        auto convertedParamType = convert(paramType);
        if (!convertedParamType)
            return nullptr;
        paramTypes.push_back(convertedParamType);
    }

    auto returnType = convert(funcType->getReturnType());
    if (!returnType)
        return nullptr;
    bool isVariadic = funcType->isVariadic();

    return typesArena.create<types::FunctionTy>(
        paramTypes, returnType, isVariadic
    );
}

} // namespace glu::clangimporter
