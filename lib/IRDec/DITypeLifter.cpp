#include "ModuleLifter.hpp"

#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/DebugInfoMetadata.h>

namespace glu::irdec {

class DITypeLifter {

    ModuleLiftingContext &_ctx;
    glu::ast::ASTContext &_context;

public:
    DITypeLifter(ModuleLiftingContext &context)
        : _ctx(context), _context(context.ast)
    {
    }

    /// @brief Lift a DIType to a GLU type
    /// @param diType The DIType to lift
    /// @return The lifted GLU type, or nullptr if the type could not be lifted
    glu::types::TypeBase *lift(llvm::DIType const *diType)
    {
        if (!diType) {
            return _context.getTypesMemoryArena().create<types::VoidTy>();
        }
        switch (diType->getMetadataID()) {
        case llvm::Metadata::MetadataKind::DIBasicTypeKind:
            return handleBasicType(llvm::cast<llvm::DIBasicType>(diType));
        case llvm::Metadata::MetadataKind::DICompositeTypeKind:
            return handleComposedTypes(
                llvm::cast<llvm::DICompositeType>(diType)
            );
        case llvm::Metadata::MetadataKind::DIDerivedTypeKind:
            return handleDerivedType(llvm::cast<llvm::DIDerivedType>(diType));
        case llvm::Metadata::MetadataKind::DISubroutineTypeKind:
            return handleSubroutineType(
                llvm::cast<llvm::DISubroutineType>(diType)
            );
        default: return nullptr;
        }
    }

    glu::types::TypeBase *
    handleBasicType(llvm::DIBasicType const *diBasicType) const
    {
        using Signedness = glu::types::IntTy::Signedness;

        auto encoding = diBasicType->getEncoding();
        auto sizeInBits = diBasicType->getSizeInBits();
        auto &arena = _context.getTypesMemoryArena();

        switch (encoding) {
        case llvm::dwarf::DW_ATE_signed:
            return arena.create<types::IntTy>(Signedness::Signed, sizeInBits);
        case llvm::dwarf::DW_ATE_unsigned:
            return arena.create<types::IntTy>(Signedness::Unsigned, sizeInBits);
        case llvm::dwarf::DW_ATE_float:
            return arena.create<types::FloatTy>(sizeInBits);
        case llvm::dwarf::DW_ATE_boolean: return arena.create<types::BoolTy>();
        case llvm::dwarf::DW_ATE_address:
            return arena.create<types::PointerTy>(
                arena.create<types::CharTy>()
            );
        case llvm::dwarf::DW_ATE_unsigned_char:
        case llvm::dwarf::DW_ATE_signed_char:
            return arena.create<types::CharTy>();
        default: return nullptr;
        }
    }

    glu::types::StructTy *
    handleStructureType(llvm::DICompositeType const *diCompositeType)
    {
        auto &astArena = _context.getASTMemoryArena();
        if (auto *structDecl = llvm::dyn_cast_if_present<ast::StructDecl>(
                _ctx.diTypeCache[diCompositeType]
            )) {
            return structDecl->getType();
        }

        std::vector<ast::FieldDecl *> fieldDecls;
        if (auto elements = diCompositeType->getElements()) {
            for (auto *elem : elements) {
                if (auto *derivedType
                    = llvm::dyn_cast<llvm::DIDerivedType>(elem)) {
                    if (derivedType->getTag() != llvm::dwarf::DW_TAG_member)
                        continue;
                    auto fieldName = derivedType->getName();
                    auto fieldType = lift(derivedType->getBaseType());
                    if (!fieldType) {
                        return nullptr;
                    }
                    fieldDecls.push_back(astArena.create<ast::FieldDecl>(
                        SourceLocation::invalid,
                        copyString(fieldName, astArena.getAllocator()),
                        fieldType, nullptr, nullptr, ast::Visibility::Public
                    ));
                }
            }
        }
        auto structDecl = astArena.create<ast::StructDecl>(
            _context, SourceLocation::invalid, nullptr,
            copyString(diCompositeType->getName(), astArena.getAllocator()),
            fieldDecls, nullptr, ast::Visibility::Public
        );
        _ctx.diTypeCache[diCompositeType] = structDecl;
        _ctx.addToNamespace(diCompositeType->getScope(), structDecl);
        return structDecl->getType();
    }

    glu::types::EnumTy *
    handleEnumType(llvm::DICompositeType const *diCompositeType)
    {
        auto &astArena = _context.getASTMemoryArena();
        if (auto *enumDecl = llvm::dyn_cast_if_present<ast::EnumDecl>(
                _ctx.diTypeCache[diCompositeType]
            )) {
            return enumDecl->getType();
        }

        std::vector<ast::FieldDecl *> enumerators;
        if (auto elements = diCompositeType->getElements()) {
            for (auto *elem : elements) {
                if (auto *enumerator
                    = llvm::dyn_cast<llvm::DIEnumerator>(elem)) {
                    auto name = enumerator->getName();
                    enumerators.push_back(astArena.create<ast::FieldDecl>(
                        SourceLocation::invalid,
                        copyString(name.str(), astArena.getAllocator()), nullptr
                    ));
                }
            }
        }
        auto enumDecl = astArena.create<ast::EnumDecl>(
            _context, SourceLocation::invalid, nullptr,
            copyString(
                diCompositeType->getName().str(), astArena.getAllocator()
            ),
            enumerators
        );
        _ctx.diTypeCache[diCompositeType] = enumDecl;
        _ctx.addToNamespace(diCompositeType->getScope(), enumDecl);
        return enumDecl->getType();
    }

    glu::types::TypeBase *
    handleComposedTypes(llvm::DICompositeType const *diCompositeType)
    {
        using namespace glu::types;

        auto &typesArena = _context.getTypesMemoryArena();
        switch (diCompositeType->getTag()) {
        case llvm::dwarf::DW_TAG_class_type:
        case llvm::dwarf::DW_TAG_structure_type:
            return handleStructureType(diCompositeType);
        case llvm::dwarf::DW_TAG_array_type: {
            if (auto elements = diCompositeType->getElements()) {
                if (elements->getNumOperands() != 2) {
                    return nullptr;
                }
                auto *subrange = llvm::dyn_cast<llvm::DISubrange>(elements[1]);
                if (!subrange) {
                    return nullptr;
                }
                auto *baseType = lift(diCompositeType->getBaseType());
                if (!baseType) {
                    return nullptr;
                }
                auto count
                    = llvm::isa<llvm::ConstantInt *>(subrange->getCount())
                    ? llvm::cast<llvm::ConstantInt *>(subrange->getCount())
                          ->getSExtValue()
                    : -1;
                if (count < 0) {
                    return nullptr;
                }
                return typesArena.create<StaticArrayTy>(
                    baseType, static_cast<size_t>(count)
                );
            }
            return nullptr;
        }
        case llvm::dwarf::DW_TAG_enumeration_type:
            return handleEnumType(diCompositeType);
        default: return nullptr;
        }
    }

    glu::types::TypeBase *
    handleDerivedType(llvm::DIDerivedType const *diDerivedType)
    {
        using namespace glu::types;

        auto &typesArena = _context.getTypesMemoryArena();
        auto tag = diDerivedType->getTag();

        switch (tag) {
        case llvm::dwarf::DW_TAG_pointer_type: {
            auto *baseType = lift(diDerivedType->getBaseType());
            if (!baseType) {
                return nullptr;
            }
            return typesArena.create<PointerTy>(baseType);
        }
        case llvm::dwarf::DW_TAG_typedef:
        case llvm::dwarf::DW_TAG_const_type:
        case llvm::dwarf::DW_TAG_volatile_type:
            return lift(diDerivedType->getBaseType());
        default: return nullptr;
        }
    }

    glu::types::TypeBase *
    handleSubroutineType(llvm::DISubroutineType const *diSubroutineType)
    {
        using namespace glu::types;

        auto &typesArena = _context.getTypesMemoryArena();
        auto types = diSubroutineType->getTypeArray();

        if (types.size() == 0) {
            return nullptr;
        }

        // First element is return type
        auto *returnType = lift(types[0]);
        if (!returnType) {
            return nullptr;
        }

        // Remaining elements are parameter types
        llvm::SmallVector<TypeBase *, 4> paramTypes;
        for (size_t i = 1; i < types.size(); ++i) {
            auto *paramType = lift(types[i]);
            if (!paramType) {
                return nullptr;
            }
            paramTypes.push_back(paramType);
        }

        return FunctionTy::create(
            typesArena.getAllocator(), paramTypes, returnType, false
        );
    }
};

glu::types::TypeBase *
lift(llvm::DIType const *diType, ModuleLiftingContext &ctx)
{
    return DITypeLifter(ctx).lift(diType);
}

} // namespace glu::irdec
