#include "ConstraintSystem.hpp"
#include <llvm/Support/Casting.h>

namespace glu::sema {

/// @brief A visitor that performs type conversion checking between two types.
///
/// This visitor traverses the source type and determines if it can be converted
/// to a target type. It handles both implicit and explicit conversions for
/// various type combinations including numeric types, pointers, arrays, and
/// composite types.
class ConversionVisitor
    : public glu::types::TypeVisitor<ConversionVisitor, bool> {
    ConstraintSystem *_system;
    glu::types::Ty _targetType;
    SystemState &_state;
    bool _isExplicit; // Whether this is an explicit conversion (checked cast)

public:
    ConversionVisitor(
        ConstraintSystem *system, glu::types::Ty targetType, SystemState &state, 
        bool isExplicit = false
    )
        : _system(system), _targetType(targetType), _state(state), _isExplicit(isExplicit)
    {
    }

    /// @brief Check if conversion is valid before visiting the type.
    /// @return true if conversion is valid and we should skip normal visiting,
    ///         false if we should continue with normal visiting.
    bool beforeVisit(glu::types::TypeBase *type)
    {
        // If target type is a type variable, unify with it
        if (auto *targetVar = llvm::dyn_cast<glu::types::TypeVariableTy>(_targetType)) {
            return _system->unify(type, targetVar, _state);
        }
        
        // For source type variables, proceed to the specific visitor to handle unification
        if (llvm::isa<glu::types::TypeVariableTy>(type)) {
            return false; // Continue to the specific visitor method
        }
        
        // Same type is always valid
        if (type == _targetType) {
            return true;
        }
        
        // Return false to indicate we should continue with the normal visit
        return false;
    }

    /// @brief Default case for types that don't have specific conversion rules.
    bool visitTypeBase(glu::types::TypeBase *type)
    {
        // Default: only identical types can be converted
        return type == _targetType;
    }

    /// @brief Handle integer type conversions.
    bool visitIntTy(glu::types::IntTy *fromInt)
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
    bool visitFloatTy(glu::types::FloatTy *fromFloat)
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
    bool visitStaticArrayTy(glu::types::StaticArrayTy *arrayType)
    {
        auto *pointerType = llvm::dyn_cast<glu::types::PointerTy>(_targetType);
        if (!pointerType)
            return false;

        // Array to pointer conversion: element types must match
        return arrayType->getDataType() == pointerType->getPointee();
    }

    /// @brief Handle pointer type conversions.
    bool visitPointerTy(glu::types::PointerTy *fromPtr)
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
    bool visitEnumTy(glu::types::EnumTy *fromEnum)
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
    bool visitFunctionTy(glu::types::FunctionTy *fromFunc)
    {
        auto *toFunc = llvm::dyn_cast<glu::types::FunctionTy>(_targetType);
        if (!toFunc)
            return false;

        // Function types must match exactly for conversions
        // (function pointer compatibility is handled elsewhere)
        return fromFunc == toFunc;
    }

    /// @brief Handle dynamic array type conversions.
    bool visitDynamicArrayTy(glu::types::DynamicArrayTy *fromArray)
    {
        auto *toArray = llvm::dyn_cast<glu::types::DynamicArrayTy>(_targetType);
        if (!toArray)
            return false;

        // Dynamic array types must match exactly
        return fromArray == toArray;
    }

    /// @brief Handle struct type conversions.
    bool visitStructTy(glu::types::StructTy *fromStruct)
    {
        auto *toStruct = llvm::dyn_cast<glu::types::StructTy>(_targetType);
        if (!toStruct)
            return false;

        // Struct types must match exactly for conversions
        return fromStruct == toStruct;
    }

    /// @brief Handle type variable conversions.
    bool visitTypeVariableTy(glu::types::TypeVariableTy *fromVar)
    {
        // For type variables, attempt unification instead of just returning true
        return _system->unify(fromVar, _targetType, _state);
    }
};

bool isValidConversion(
    ConstraintSystem *system,
    glu::types::Ty fromType, glu::types::Ty toType, SystemState &state,
    bool isExplicit
)
{
    // Same type is always valid
    if (fromType == toType) {
        return true;
    }

    // Use the conversion visitor for systematic conversion checking
    ConversionVisitor visitor(system, toType, state, isExplicit);
    return visitor.visit(fromType);
}

} // namespace glu::sema