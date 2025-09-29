#include "ConstraintSystem.hpp"
#include <llvm/Support/Casting.h>

namespace glu::sema {

/// @brief A visitor that performs type conversion checking between two types.
///
/// This visitor traverses the source type and determines if it can be converted
/// to a target type. It handles both implicit and explicit conversions for
/// various type combinations including numeric types, pointers, arrays, and
/// composite types.
class ConversionVisitor : public types::TypeVisitor<ConversionVisitor, bool> {
    ConstraintSystem *_system;
    types::Ty _targetType;
    SystemState &_state;
    bool _isExplicit; // Whether this is an explicit conversion (checked cast)

public:
    ConversionVisitor(
        ConstraintSystem *system, types::Ty targetType, SystemState &state,
        bool isExplicit = false
    )
        : _system(system)
        , _targetType(targetType)
        , _state(state)
        , _isExplicit(isExplicit)
    {
    }

    /// @brief Check if conversion is valid before visiting the type.
    /// @return true if conversion is valid and we should skip normal visiting,
    ///         false if we should continue with normal visiting.
    bool beforeVisit(types::TypeBase *type)
    {
        // If target type is a type variable, unify with it
        if (auto *targetVar
            = llvm::dyn_cast<types::TypeVariableTy>(_targetType)) {
            return _system->unify(type, targetVar, _state);
        }

        // For source type variables, proceed to the specific visitor to handle
        // unification
        if (llvm::isa<types::TypeVariableTy>(type)) {
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
    bool visitTypeBase(types::TypeBase *type)
    {
        // Default: only identical types can be converted
        return type == _targetType;
    }

    /// @brief Handle integer type conversions.
    bool visitIntTy(types::IntTy *fromInt)
    {
        auto *toInt = llvm::dyn_cast<types::IntTy>(_targetType);
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
        if (auto *toPtr = llvm::dyn_cast<types::PointerTy>(_targetType)) {
            if (_isExplicit) {
                // For type variables nested in the pointer target, unify
                if (llvm::isa<types::TypeVariableTy>(toPtr->getPointee())) {
                    return _system->unify(fromInt, _targetType, _state);
                }
                return true;
            }
            return false;
        }

        // Integer to type variable - unify first
        if (llvm::isa<types::TypeVariableTy>(_targetType)) {
            return _system->unify(fromInt, _targetType, _state);
        }

        // Integer to float conversion (explicit only)
        if (llvm::isa<types::FloatTy>(_targetType)) {
            return _isExplicit;
        }

        if (llvm::isa<types::BoolTy>(_targetType)) {
            return _isExplicit;
        }

        if (llvm::isa<types::CharTy>(_targetType)) {
            if (fromInt->getBitWidth() == 8) {
                return true;
            }
            return _isExplicit;
        }

        // Integer to enum conversion (explicit only)
        if (llvm::isa<types::EnumTy>(_targetType)) {
            if (_isExplicit) {
                // For type variables nested in the enum type, unify
                if (llvm::isa<types::TypeVariableTy>(_targetType)) {
                    return _system->unify(fromInt, _targetType, _state);
                }
                return true;
            }
            return false;
        }

        return false;
    }

    /// @brief Handle float type conversions.
    bool visitFloatTy(types::FloatTy *fromFloat)
    {
        if (auto *toFloat = llvm::dyn_cast<types::FloatTy>(_targetType)) {
            if (fromFloat == toFloat)
                return true;

            if (fromFloat->getBitWidth() <= toFloat->getBitWidth()) {
                return true;
            }

            return _isExplicit;
        }

        if (llvm::isa<types::TypeVariableTy>(_targetType)) {
            return _system->unify(fromFloat, _targetType, _state);
        }

        if (llvm::isa<types::IntTy>(_targetType)) {
            return _isExplicit;
        }

        return false;
    }

    bool visitStaticArrayTy(types::StaticArrayTy *arrayType)
    {
        if (auto *pointerType = llvm::dyn_cast<types::PointerTy>(_targetType)) {
            return _system->unify(
                arrayType->getDataType(), pointerType->getPointee(), _state
            );
        }

        return _system->unify(arrayType, _targetType, _state);
    }

    /// @brief Handle pointer type conversions.
    bool visitPointerTy(types::PointerTy *fromPtr)
    {
        // Pointer to pointer conversion
        if (auto *toPtr = llvm::dyn_cast<types::PointerTy>(_targetType)) {
            // Same pointer type
            if (fromPtr == toPtr)
                return true;

            // Allow explicit pointer-to-pointer conversions
            if (_isExplicit) {
                // For type variables nested in the pointer pointee types, unify
                // them
                if (llvm::isa<types::TypeVariableTy>(fromPtr->getPointee())
                    || llvm::isa<types::TypeVariableTy>(toPtr->getPointee())) {
                    return _system->unify(
                        fromPtr->getPointee(), toPtr->getPointee(), _state
                    );
                }
                // For concrete types, allow explicit conversion without
                // unification
                return true;
            }

            // Implicit pointer conversions are more restrictive
            // For now, only allow compatible pointee types (including type
            // variables)
            return _system->unify(
                fromPtr->getPointee(), toPtr->getPointee(), _state
            );
        }

        // Pointer to integer conversion (explicit only)
        if (llvm::isa<types::IntTy>(_targetType)) {
            if (_isExplicit) {
                // For type variables, unify
                if (llvm::isa<types::TypeVariableTy>(_targetType)) {
                    return _system->unify(fromPtr, _targetType, _state);
                }
                return true;
            }
            return false;
        }

        return false;
    }

    /// @brief Handle enum type conversions.
    bool visitEnumTy(types::EnumTy *fromEnum)
    {
        // Enum to integer conversion
        if (llvm::isa<types::IntTy>(_targetType)) {
            if (_isExplicit) {
                // For type variables, unify
                if (llvm::isa<types::TypeVariableTy>(_targetType)) {
                    return _system->unify(fromEnum, _targetType, _state);
                }
                return true;
            }
            return false;
        }

        // Same enum type
        if (llvm::isa<types::EnumTy>(_targetType)) {
            return _system->unify(fromEnum, _targetType, _state);
        }

        return false;
    }

    /// @brief Handle function type conversions.
    bool visitFunctionTy(types::FunctionTy *fromFunc)
    {
        auto *toFunc = llvm::dyn_cast<types::FunctionTy>(_targetType);
        if (!toFunc)
            return false;

        if (toFunc->isCVariadic()) {
            if (fromFunc->getParameterCount() < toFunc->getParameterCount())
                return false;
            for (size_t size = 0; size < toFunc->getParameterCount(); size++) {
                if (!_system->unify(
                        fromFunc->getParameters()[size],
                        toFunc->getParameters()[size], _state
                    ))
                    return false;
            }
            return _system->unify(
                fromFunc->getReturnType(), toFunc->getReturnType(), _state
            );
        }
        // Function types must match exactly for conversions
        // (function pointer compatibility is handled elsewhere)
        // Handle type variables in function signatures by unifying
        return _system->unify(fromFunc, toFunc, _state);
    }

    /// @brief Handle dynamic array type conversions.
    bool visitDynamicArrayTy(types::DynamicArrayTy *fromArray)
    {
        auto *toArray = llvm::dyn_cast<types::DynamicArrayTy>(_targetType);
        if (!toArray)
            return false;

        // Dynamic array types must match exactly, but unify to handle type
        // variables
        if (auto *targetArray
            = llvm::dyn_cast<types::DynamicArrayTy>(_targetType)) {
            return _system->unify(fromArray, targetArray, _state);
        }
        return false;
    }

    /// @brief Handle struct type conversions.
    bool visitStructTy(types::StructTy *fromStruct)
    {
        auto *toStruct = llvm::dyn_cast<types::StructTy>(_targetType);
        if (!toStruct)
            return false;

        // Struct types must match exactly for conversions, but unify to handle
        // type variables
        if (auto *targetStruct = llvm::dyn_cast<types::StructTy>(_targetType)) {
            return _system->unify(fromStruct, targetStruct, _state);
        }
        return false;
    }

    bool visitBoolTy(types::BoolTy *fromBool)
    {
        if (llvm::isa<types::BoolTy>(_targetType)) {
            return true;
        }

        // Bool to type variable - unify first
        if (llvm::isa<types::TypeVariableTy>(_targetType)) {
            return _system->unify(fromBool, _targetType, _state);
        }

        if (llvm::isa<types::IntTy>(_targetType)) {
            return _isExplicit;
        }

        return false;
    }

    bool visitCharTy(types::CharTy *fromChar)
    {
        if (llvm::isa<types::IntTy>(_targetType)) {
            return true;
        }

        if (llvm::isa<types::CharTy>(_targetType)) {
            return true;
        }

        return false;
    }

    /// @brief Handle type variable conversions.
    bool visitTypeVariableTy(types::TypeVariableTy *fromVar)
    {
        // For type variables, attempt unification instead of just returning
        // true
        return _system->unify(fromVar, _targetType, _state);
    }

    bool visitNullTy(types::NullTy *ty)
    {
        // Unify null type with target type
        if (llvm::isa<types::PointerTy>(_targetType))
            return true;
        return _system->unify(ty, _targetType, _state);
    }
};

bool ConstraintSystem::isValidConversion(
    types::Ty fromType, types::Ty toType, SystemState &state, bool isExplicit
)
{
    // Same type is always valid
    if (fromType == toType) {
        return true;
    }

    // Use the conversion visitor for systematic conversion checking
    ConversionVisitor visitor(this, toType, state, isExplicit);
    return visitor.visit(fromType);
}

} // namespace glu::sema
