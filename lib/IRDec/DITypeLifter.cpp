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
    default: return nullptr;
    }
}

} // namespace glu::irdec
