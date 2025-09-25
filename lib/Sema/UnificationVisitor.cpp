#include "ConstraintSystem.hpp"

namespace glu::sema {

/// @brief A visitor that performs structural unification between two types.
///
/// This visitor traverses the first type and attempts to unify it with a target
/// type. It handles structural unification for composite types like pointers,
/// functions, and arrays, delegating to recursive unification for their
/// components.
class UnificationVisitor
    : public glu::types::TypeVisitor<UnificationVisitor, bool> {
    ConstraintSystem *_system;
    glu::types::Ty _targetType;
    SystemState &_state;

public:
    UnificationVisitor(
        ConstraintSystem *system, glu::types::Ty targetType, SystemState &state
    )
        : _system(system), _targetType(targetType), _state(state)
    {
    }

    bool visitTypeBase(glu::types::TypeBase *type)
    {
        // For primitive types and other types, just check equality
        return type == _targetType;
    }

    bool visitPointerTy(glu::types::PointerTy *type)
    {
        auto *targetPtr = llvm::dyn_cast<glu::types::PointerTy>(_targetType);
        if (!targetPtr)
            return false;
        return _system->unify(
            type->getPointee(), targetPtr->getPointee(), _state
        );
    }

    bool visitFunctionTy(glu::types::FunctionTy *type)
    {
        auto *targetFn = llvm::dyn_cast<glu::types::FunctionTy>(_targetType);
        if (!targetFn)
            return false;

        if (type->getParameterCount() != targetFn->getParameterCount())
            return false;

        // Unify return types
        if (!_system->unify(
                type->getReturnType(), targetFn->getReturnType(), _state
            ))
            return false;

        // Unify parameter types
        for (size_t i = 0; i < type->getParameterCount(); ++i) {
            if (!_system->unify(
                    type->getParameter(i), targetFn->getParameter(i), _state
                ))
                return false;
        }
        return true;
    }

    bool visitStaticArrayTy(glu::types::StaticArrayTy *type)
    {
        auto *targetArr
            = llvm::dyn_cast<glu::types::StaticArrayTy>(_targetType);
        if (!targetArr)
            return false;

        if (type->getSize() != targetArr->getSize())
            return false;

        return _system->unify(
            type->getDataType(), targetArr->getDataType(), _state
        );
    }

    bool visitDynamicArrayTy(glu::types::DynamicArrayTy *type)
    {
        auto *targetArr
            = llvm::dyn_cast<glu::types::DynamicArrayTy>(_targetType);
        if (!targetArr)
            return false;
        return _system->unify(
            type->getDataType(), targetArr->getDataType(), _state
        );
    }

    bool visitStructTy(glu::types::StructTy *type)
    {
        auto *targetStruct = llvm::dyn_cast<glu::types::StructTy>(_targetType);
        if (!targetStruct)
            return false;

        if (type->getFieldCount() != targetStruct->getFieldCount())
            return false;

        for (size_t i = 0; i < type->getFieldCount(); ++i) {
            auto const &fieldA = type->getField(i);
            auto const &fieldB = targetStruct->getField(i);
            if (fieldA->getName() != fieldB->getName())
                return false;
            if (!_system->unify(fieldA->getType(), fieldB->getType(), _state))
                return false;
        }
        return false;
    }

    // Add more cases for other composite types (structs, etc.)
};

bool ConstraintSystem::unify(
    glu::types::Ty first, glu::types::Ty second, SystemState &state
)
{
    // Apply substitutions
    first = substitute(first, state.typeBindings, _context);
    second = substitute(second, state.typeBindings, _context);

    if (first == second)
        return true;

    // Handle type variables
    if (auto *firstVar = llvm::dyn_cast<glu::types::TypeVariableTy>(first)) {
        if (occursCheck(firstVar, second, state.typeBindings))
            return false;
        state.typeBindings[firstVar] = second;
        return true;
    }

    if (auto *secondVar = llvm::dyn_cast<glu::types::TypeVariableTy>(second)) {
        if (occursCheck(secondVar, first, state.typeBindings))
            return false;
        state.typeBindings[secondVar] = first;
        return true;
    }

    // Use the visitor for structural unification
    UnificationVisitor visitor(this, second, state);
    return visitor.visit(first);
}

} // namespace glu::sema
