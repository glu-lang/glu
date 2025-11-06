#include "TypeTranslator.hpp"
#include "AST/Decls.hpp"
#include <llvm/Support/Casting.h>

namespace glu::gilgen {

gil::Type TypeTranslator::visitIntTy(types::IntTy *type)
{
    // Calculate size in bytes from bit width
    unsigned size = (type->getBitWidth() + 7) / 8;

    return gil::Type(size, size, false, type);
}

gil::Type TypeTranslator::visitFloatTy(types::FloatTy *type)
{
    unsigned size = type->getBitWidth() / 8;

    return gil::Type(size, size, false, type);
}

gil::Type TypeTranslator::visitBoolTy(types::BoolTy *type)
{
    return gil::Type(1, 1, false, type);
}

gil::Type TypeTranslator::visitCharTy(types::CharTy *type)
{
    return gil::Type(1, 1, false, type);
}

gil::Type TypeTranslator::visitVoidTy(types::VoidTy *type)
{
    // Void represents absence of value - zero size but with alignment of 1
    return gil::Type(0, 1, false, type);
}

gil::Type TypeTranslator::visitPointerTy(types::PointerTy *type)
{
    // Pointers are typically 8 bytes on 64-bit systems
    // Note: We could potentially make this platform-dependent
    return gil::Type(8, 8, false, type);
}

gil::Type TypeTranslator::visitFunctionTy(types::FunctionTy *type)
{
    // Function types are represented as function pointers
    // Same size as a regular pointer on the target platform
    return gil::Type(8, 8, false, type);
}

gil::Type TypeTranslator::visitStructTy(types::StructTy *type)
{
    // For structures, we need to calculate size and alignment based on fields
    // This is a simple implementation that considers field alignment
    unsigned size = 0;
    unsigned alignment = 1;

    // Iterate through all fields in the structure
    for (auto const &field : type->getFields()) {
        // Recursively translate the field's type
        gil::Type fieldType = visit(field->getType());

        // Apply alignment padding if necessary
        size = (size + fieldType.getAlignment() - 1) / fieldType.getAlignment()
            * fieldType.getAlignment();
        size += fieldType.getSize();

        // Structure's alignment is the max alignment of its fields
        alignment = std::max(alignment, fieldType.getAlignment());
    }

    // Round the total size to the structure's alignment
    size = (size + alignment - 1) / alignment * alignment;

    return gil::Type(size, alignment, false, type);
}

gil::Type TypeTranslator::visitStaticArrayTy(types::StaticArrayTy *type)
{
    // For static arrays, calculate total size based on element type and count
    gil::Type elemType = visit(type->getDataType());
    unsigned size = elemType.getSize() * type->getSize();

    // Array alignment is the same as element alignment
    return gil::Type(size, elemType.getAlignment(), false, type);
}

gil::Type TypeTranslator::visitDynamicArrayTy(types::DynamicArrayTy *type)
{
    // Dynamic arrays are typically represented as a structure containing:
    // - A pointer to the data (8 bytes)
    // - A size field (8 bytes)
    return gil::Type(16, 8, false, type);
}

gil::Type TypeTranslator::visitTypeAliasTy(types::TypeAliasTy *type)
{
    return visit(type->getWrappedType());
}

gil::Type TypeTranslator::visitEnumTy(types::EnumTy *type)
{
    // We represent enums as 4-byte integers by default
    // This is a simplification - in a complete implementation, we would:
    // 1. Determine the minimum bit width needed to represent all possible
    // values
    // 2. Round up to the nearest standard integer size (8, 16, 32, 64 bits)

    // For now, use a standard 32-bit representation
    return gil::Type(4, 4, false, type);
}

gil::Type TypeTranslator::visitTemplateParamTy(types::TemplateParamTy *type)
{
    // Treat template parameters similarly to type variables — use placeholder.
    return gil::Type(8, 8, false, type);
}

gil::Type TypeTranslator::visitTypeVariableTy(types::TypeVariableTy *type)
{
    // Type variables represent types that are not yet known.
    // For compilation, we need to use a placeholder representation.
    // Using a generic pointer size (8 bytes on a 64-bit system) as a default.
    return gil::Type(8, 8, false, type);
}

gil::Type TypeTranslator::visitUnresolvedNameTy(types::UnresolvedNameTy *type)
{
    // Unresolved name types represent types that haven't been resolved yet.
    // Similar to type variables, we use a placeholder representation.
    // Note: In a complete implementation, this should probably trigger an error
    // or be resolved at an earlier stage.
    return gil::Type(8, 8, false, type);
}

gil::Type TypeTranslator::visitNullTy(types::NullTy *ty)
{
    return gil::Type(8, 8, false, ty);
}

} // namespace glu::gilgen
