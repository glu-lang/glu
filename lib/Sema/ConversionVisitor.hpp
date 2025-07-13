#ifndef GLU_SEMA_CONVERSIONVISITOR_HPP
#define GLU_SEMA_CONVERSIONVISITOR_HPP

#include "AST/Types.hpp"

namespace glu::sema {

// Forward declarations
struct SystemState;
class ConstraintSystem;

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
    );

    /// @brief Check if conversion is valid before visiting the type.
    /// @return true if conversion is valid and we should skip normal visiting,
    ///         false if we should continue with normal visiting.
    bool beforeVisit(glu::types::TypeBase *type);

    /// @brief Default case for types that don't have specific conversion rules.
    bool visitTypeBase(glu::types::TypeBase *type);

    /// @brief Handle integer type conversions.
    bool visitIntTy(glu::types::IntTy *fromInt);

    /// @brief Handle float type conversions.
    bool visitFloatTy(glu::types::FloatTy *fromFloat);

    /// @brief Handle static array to pointer conversions.
    bool visitStaticArrayTy(glu::types::StaticArrayTy *arrayType);

    /// @brief Handle pointer type conversions.
    bool visitPointerTy(glu::types::PointerTy *fromPtr);

    /// @brief Handle enum type conversions.
    bool visitEnumTy(glu::types::EnumTy *fromEnum);

    /// @brief Handle function type conversions.
    bool visitFunctionTy(glu::types::FunctionTy *fromFunc);

    /// @brief Handle dynamic array type conversions.
    bool visitDynamicArrayTy(glu::types::DynamicArrayTy *fromArray);

    /// @brief Handle struct type conversions.
    bool visitStructTy(glu::types::StructTy *fromStruct);

    /// @brief Handle type variable conversions.
    bool visitTypeVariableTy(glu::types::TypeVariableTy *fromVar);
};

} // namespace glu::sema

#endif // GLU_SEMA_CONVERSIONVISITOR_HPP