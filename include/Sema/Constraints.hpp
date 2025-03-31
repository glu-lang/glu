#ifndef GLU_SEMA_CONSTRAINTS_HPP
#define GLU_SEMA_CONSTRAINTS_HPP

#include "AST/Exprs.hpp"
#include "GIL/Type.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/TrailingObjects.h>
namespace glu::sema {

enum class ConstraintKind : char {

    /// The two types must be bound to the same type. This is the only
    /// truly symmetric constraint.
    Bind,
    /// The two types must be bound to the same type, dropping
    /// lvalueness when comparing a type variable to a type.
    Equal,
    /// Binds the first type to the element type of the second type.
    BindToPointerType,
    /// The first type is convertible to the second type.
    Conversion,
    /// The first type is the element of an argument tuple that is
    /// convertible to the second type (which represents the corresponding
    /// parameter type).
    ArgumentConversion,
    /// The first type is convertible to the second type.
    OperatorArgumentConversion,
    /// A checked cast from the first type to the second.
    CheckedCast,

    // TODO: Remove this constraint kind if needed.
    /// Both types are function types. The first function type's
    /// input is the value being passed to the function and its output
    /// is a type variable that describes the output. The second
    /// function type is expected to become a function type. Note, we
    /// do not require the function type attributes to match.
    ApplicableFunction,

    /// Binds the left-hand type to a particular overload choice.
    BindOverload,
    /// The first type has a member with the given name, and the
    /// type of that member, when referenced as a value, is the second type.
    ValueMember,
    /// The first type (which is implicit) has a member with the given
    /// name, and the type of that member, when referenced as a value, is the
    /// second type.
    UnresolvedValueMember,
    /// The first type can be defaulted to the second (which currently
    /// cannot be dependent).  This is more like a type property than a
    /// relational constraint.
    Defaultable,

    /// A disjunction constraint that specifies that one or more of the
    /// stored constraints must hold.
    Disjunction,

    /// A conjunction constraint that specifies that all of the stored
    /// constraints must hold.
    Conjunction,

    /// If there is no contextual info e.g. `_ = { 42 }` default first type
    /// to a second type. This is effectively a `Defaultable` constraint
    /// which one significant difference:
    ///
    /// - Handled specially by binding inference, specifically contributes
    ///   to the bindings only if there are no contextual types available.
    FallbackType,

    /// Represents explicit generic arguments provided for a reference to
    /// a declaration.
    ///
    /// The first type is the type variable describing the bound type of
    /// an overload. The second type is a PackType containing the explicit
    /// generic arguments.
    ExplicitGenericArguments,
    /// The first type is a l-value type whose object type is the second type.
    LValueObject,

};

/// Classification of the different kinds of constraints.
enum class ConstraintClassification : char {
    /// A relational constraint, which relates two types.
    Relational,

    /// A member constraint, which names a member of a type and assigns
    /// it a reference type.
    Member,

    /// A property of a single type, such as whether it is defaultable to
    /// a particular type.
    TypeProperty,

    /// A disjunction constraint.
    Disjunction,

    /// A conjunction constraint.
    Conjunction,

    /// An element of a closure/function body.
    SyntacticElement,
};

enum class ConversionRestrictionKind {
    /// Deep equality comparison.
    DeepEquality,
    /// Array-to-pointer conversion.
    ArrayToPointer,
    /// String-to-pointer conversion.
    StringToPointer,
    /// Pointer-to-pointer conversion.
    PointerToPointer,
};

/// A constraint between two type variables.
class Constraint final
    : public llvm::ilist_node<Constraint>,
      private llvm::TrailingObjects<Constraint, glu::types::TypeVariableTy *> {
    using TrailingArgs
        = llvm::TrailingObjects<Constraint, glu::types::TypeVariableTy *>;
    friend TrailingArgs;

    union {
        struct {
            /// The first type.
            glu::gil::Type First;

            /// The second type.
            glu::gil::Type Second;

            /// The third type, if any.
            glu::gil::Type Third;
        } Types;

        struct {
            /// The type of the base.
            glu::gil::Type First;

            /// The type of the member.
            glu::gil::Type Second;
            union {
                /// If non-null, the name of a member of the first type is that
                /// being related to the second type.
                ///
                /// Used for ValueMember an UnresolvedValueMember constraints.
                glu::ast::RefExpr *Ref;

                /// If non-null, the member being referenced.
                ///
                /// Used for ValueWitness constraints.
                glu::ast::*Value;
            } Member;
        } Member;

        /// The set of constraints for a disjunction.
        llvm::ArrayRef<Constraint *> Nested;
    };

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINTS_HPP
