#include "DITypeLifter.hpp"
#include "AST/Decls.hpp"
#include <llvm/BinaryFormat/Dwarf.h>

namespace glu::irdec {

glu::types::TypeBase *DITypeLifter::handleBasicType(
    glu::InternedMemoryArena<glu::types::TypeBase> &arena,
    llvm::DIBasicType const *diBasicType
) const
{
    using Signedness = glu::types::IntTy::Signedness;

    auto encoding = diBasicType->getEncoding();
    auto sizeInBits = diBasicType->getSizeInBits();

    switch (encoding) {
    case llvm::dwarf::DW_ATE_signed:
        return arena.create<types::IntTy>(Signedness::Signed, sizeInBits);
    case llvm::dwarf::DW_ATE_unsigned:
        return arena.create<types::IntTy>(Signedness::Unsigned, sizeInBits);
    case llvm::dwarf::DW_ATE_float:
        return arena.create<types::FloatTy>(sizeInBits);
    case llvm::dwarf::DW_ATE_boolean: return arena.create<types::BoolTy>();
    case llvm::dwarf::DW_ATE_address:
        return arena.create<types::PointerTy>(arena.create<types::CharTy>());
    default: return nullptr;
    }
}

glu::types::TypeBase *DITypeLifter::handleComposedTypes(
    glu::InternedMemoryArena<glu::types::TypeBase> &typesArena,
    glu::TypedMemoryArena<glu::ast::ASTNode> &astArena,
    llvm::DICompositeType const *diCompositeType
) const
{
    using namespace glu::types;

    auto tag = diCompositeType->getTag();
    switch (tag) {
    case llvm::dwarf::DW_TAG_structure_type: {
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
                        copyString(fieldName.str(), astArena.getAllocator()),
                        fieldType
                    ));
                }
            }
        }
        auto structDecl = astArena.create<ast::StructDecl>(
            _context, SourceLocation::invalid, nullptr,
            copyString(
                diCompositeType->getName().str(), astArena.getAllocator()
            ),
            fieldDecls
        );
        return typesArena.create<StructTy>(structDecl);
    }
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
            auto count = subrange->getCount().dyn_cast<llvm::ConstantInt *>()
                ? subrange->getCount()
                      .dyn_cast<llvm::ConstantInt *>()
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
    case llvm::dwarf::DW_TAG_pointer_type: {
        auto *baseType = lift(diCompositeType->getBaseType());
        if (!baseType) {
            return nullptr;
        }
        return typesArena.create<PointerTy>(baseType);
    }
    case llvm::dwarf::DW_TAG_typedef: {
        auto *baseType = lift(diCompositeType->getBaseType());
        if (!baseType) {
            return nullptr;
        }
        return baseType;
    }
    case llvm::dwarf::DW_TAG_enumeration_type: {
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
        return typesArena.create<EnumTy>(enumDecl);
    }
    default: return nullptr;
    }
}

glu::types::TypeBase *DITypeLifter::lift(llvm::DIType const *diType) const
{
    auto &typesArena = _context.getTypesMemoryArena();
    auto &astArena = _context.getASTMemoryArena();

    if (!diType) {
        return nullptr;
    }
    switch (diType->getMetadataID()) {
    case llvm::Metadata::MetadataKind::DIBasicTypeKind:
        return handleBasicType(
            typesArena, llvm::cast<llvm::DIBasicType>(diType)
        );
    case llvm::Metadata::MetadataKind::DICompositeTypeKind:
        return handleComposedTypes(
            typesArena, astArena, llvm::cast<llvm::DICompositeType>(diType)
        );
    default: return nullptr;
    }
}

} // namespace glu::irdec
