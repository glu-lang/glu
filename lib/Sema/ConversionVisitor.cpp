#include "ConversionVisitor.hpp"
#include "ConstraintSystem.hpp"
#include <llvm/Support/Casting.h>

namespace glu::sema {

ConversionVisitor::ConversionVisitor(
    glu::types::Ty targetType, SystemState &state, bool isExplicit
)
    : _targetType(targetType), _state(state), _isExplicit(isExplicit)
{
}

/// @brief Default case for types that don't have specific conversion rules.
bool ConversionVisitor::visitTypeBase(glu::types::TypeBase *type)
{
    // Default: only identical types can be converted
    return type == _targetType;
}

/// @brief Handle integer type conversions.
bool ConversionVisitor::visitIntTy(glu::types::IntTy *fromInt)
{
    auto *toInt = llvm::dyn_cast<glu::types::IntTy>(_targetType);
    if (toInt) {
        // Same integer type
        if (fromInt == toInt)
            return true;

        // Allow implicit widening conversions (smaller to larger)
        if (fromInt->getBitWidth() <= toInt->getBitWidth()) {
            return true;
        }

        // Allow explicit narrowing conversions in checked casts
        if (_isExplicit) {
            return true;
        }

        // Implicit narrowing is not allowed
        return false;
    }

    // Integer to pointer conversion (explicit only)
    if (auto *toPtr = llvm::dyn_cast<glu::types::PointerTy>(_targetType)) {
        return _isExplicit;
    }

    // Integer to enum conversion (explicit only)
    if (auto *toEnum = llvm::dyn_cast<glu::types::EnumTy>(_targetType)) {
        return _isExplicit;
    }

    return false;
}

/// @brief Handle float type conversions.
bool ConversionVisitor::visitFloatTy(glu::types::FloatTy *fromFloat)
{
    auto *toFloat = llvm::dyn_cast<glu::types::FloatTy>(_targetType);
    if (!toFloat)
        return false;

    // Same float type
    if (fromFloat == toFloat)
        return true;

    // Allow implicit widening of floats (smaller to larger)
    if (fromFloat->getBitWidth() <= toFloat->getBitWidth()) {
        return true;
    }

    // Allow explicit narrowing conversions in checked casts
    if (_isExplicit) {
        return true;
    }

    // Implicit narrowing is not allowed
    return false;
}

/// @brief Handle static array to pointer conversions.
bool ConversionVisitor::visitStaticArrayTy(glu::types::StaticArrayTy *arrayType)
{
    auto *pointerType = llvm::dyn_cast<glu::types::PointerTy>(_targetType);
    if (!pointerType)
        return false;

    // Array to pointer conversion: element types must match
    return arrayType->getDataType() == pointerType->getPointee();
}

/// @brief Handle pointer type conversions.
bool ConversionVisitor::visitPointerTy(glu::types::PointerTy *fromPtr)
{
    // Pointer to pointer conversion
    if (auto *toPtr = llvm::dyn_cast<glu::types::PointerTy>(_targetType)) {
        // Same pointer type
        if (fromPtr == toPtr)
            return true;

        // Allow explicit pointer-to-pointer conversions
        if (_isExplicit) {
            return true;
        }

        // Implicit pointer conversions are more restrictive
        // For now, only allow identical pointee types
        return fromPtr->getPointee() == toPtr->getPointee();
    }

    // Pointer to integer conversion (explicit only)
    if (auto *toInt = llvm::dyn_cast<glu::types::IntTy>(_targetType)) {
        return _isExplicit;
    }

    return false;
}

/// @brief Handle enum type conversions.
bool ConversionVisitor::visitEnumTy(glu::types::EnumTy *fromEnum)
{
    // Enum to integer conversion
    if (auto *toInt = llvm::dyn_cast<glu::types::IntTy>(_targetType)) {
        return _isExplicit;
    }

    // Same enum type
    if (auto *toEnum = llvm::dyn_cast<glu::types::EnumTy>(_targetType)) {
        return fromEnum == toEnum;
    }

    return false;
}

/// @brief Handle function type conversions.
bool ConversionVisitor::visitFunctionTy(glu::types::FunctionTy *fromFunc)
{
    auto *toFunc = llvm::dyn_cast<glu::types::FunctionTy>(_targetType);
    if (!toFunc)
        return false;

    // Function types must match exactly for conversions
    // (function pointer compatibility is handled elsewhere)
    return fromFunc == toFunc;
}

/// @brief Handle dynamic array type conversions.
bool ConversionVisitor::visitDynamicArrayTy(glu::types::DynamicArrayTy *fromArray)
{
    auto *toArray = llvm::dyn_cast<glu::types::DynamicArrayTy>(_targetType);
    if (!toArray)
        return false;

    // Dynamic array types must match exactly
    return fromArray == toArray;
}

/// @brief Handle struct type conversions.
bool ConversionVisitor::visitStructTy(glu::types::StructTy *fromStruct)
{
    auto *toStruct = llvm::dyn_cast<glu::types::StructTy>(_targetType);
    if (!toStruct)
        return false;

    // Struct types must match exactly for conversions
    return fromStruct == toStruct;
}

/// @brief Handle type variable conversions.
bool ConversionVisitor::visitTypeVariableTy(glu::types::TypeVariableTy *fromVar)
{
    // Type variables always allow conversion - they will be unified later
    return true;
}

/// @brief Check if conversion is valid before visiting the type.
bool ConversionVisitor::beforeVisit(glu::types::TypeBase *type)
{
    // Type variables always allow conversion
    if (llvm::isa<glu::types::TypeVariableTy>(type) || 
        llvm::isa<glu::types::TypeVariableTy>(_targetType)) {
        return true;
    }
    
    // Same type is always valid
    if (type == _targetType) {
        return true;
    }
    
    // Return false to indicate we should continue with the normal visit
    return false;
}

} // namespace glu::sema