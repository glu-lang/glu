#ifndef GLU_SEMA_CONSTRAINTS_HPP
#define GLU_SEMA_CONSTRAINTS_HPP

#include "AST/Exprs.hpp"
#include "GIL/Type.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::sema {

///
/// @enum ConstraintKind
/// @brief Describes the different kinds of type constraints.
///
enum class ConstraintKind : char {
    Bind, ///< The two types must be bound to the same type.
    Equal, ///< Like Bind, but ignores lvalueness.
    BindToPointerType, ///< First type is element type of second (pointer).
    Conversion, ///< First type is convertible to the second.
    ArgumentConversion, ///< Conversion for function arguments.
    OperatorArgumentConversion, ///< Conversion for operator arguments.
    CheckedCast, ///< Checked cast from first to second type.
    BindOverload, ///< Binds to a specific overload.
    ValueMember, ///< First type has a value member of second type.
    UnresolvedValueMember, ///< Like ValueMember, but implicit base.
    Defaultable, ///< First type can default to second.
    Disjunction, ///< One or more constraints must hold.
    Conjunction, ///< All constraints must hold.
    FallbackType, ///< Used when no contextual type exists.
    ExplicitGenericArguments, ///< Explicit generic args for overload.
    LValueObject ///< First is l-value, second is object type.
};

///
/// @enum ConstraintClassification
/// @brief Classification of different kinds of constraints.
///
enum class ConstraintClassification : char {
    Relational, ///< Binary relation between types.
    Member, ///< Involves member access.
    TypeProperty, ///< Property on a single type.
    Disjunction, ///< Disjunction group.
    Conjunction, ///< Conjunction group.
    SyntacticElement ///< Related to a body or expression node.
};

///
/// @enum ConversionRestrictionKind
/// @brief Specifies a more precise kind of conversion restriction.
///
enum class ConversionRestrictionKind {
    DeepEquality, ///< Deep structural equality.
    ArrayToPointer, ///< Array to pointer conversion.
    StringToPointer, ///< String to pointer conversion.
    PointerToPointer ///< Pointer to pointer conversion.
};

///
/// @class Constraint
/// @brief Represents a constraint between types or variables.
///
/// Constraints express relations between type variables or concrete types
/// in the type system. They are used in type inference and checking.
///
class Constraint final
    : public llvm::ilist_node<Constraint>,
      private llvm::TrailingObjects<
          Constraint, glu::types::TypeVariableTy *, glu::ast::DeclBase> {

    using TrailingArgs
        = llvm::TrailingObjects<Constraint, glu::types::TypeVariableTy *>;
    friend TrailingArgs;

    ConstraintKind Kind; ///< Kind of constraint.

    ConversionRestrictionKind
        Restriction : 8; ///< Optional conversion restriction.

    unsigned HasFix : 1; ///< Whether a fix is allocated.
    unsigned NumTypeVariables : 11; ///< Number of type variables involved.
    unsigned HasDeclContext : 1; ///< Whether a DeclContext is present.
    unsigned HasRestriction : 1; ///< Whether Restriction is valid.
    unsigned IsActive : 1; ///< Is this constraint currently active.
    unsigned IsDisabled : 1; ///< Whether this constraint is disabled.
    unsigned RememberChoice : 1; ///< Should solver record disjunction choice.
    unsigned IsFavored : 1; ///< Preferred constraint for disjunctions.
    unsigned isDiscarded : 1; ///< Whether the result is unused.

    glu::ast::ASTNode
        *Locator; ///< Where in the AST this constraint comes from.

    union {
        struct {
            glu::gil::Type First; ///< First type involved.
            glu::gil::Type Second; ///< Second type involved.
            glu::gil::Type Third; ///< Optional third type.
        } Types;

        struct {
            glu::gil::Type First; ///< Base type.
            glu::gil::Type Second; ///< Member type.
            glu::ast::StructMemberExpr *structMember; ///< Member node.
        } Member;

        llvm::ArrayRef<Constraint *> Nested; ///< Nested constraints.

        struct {
            glu::gil::Type First; ///< Overload target type.
        } Overload;

        struct {
            glu::ast::ASTNode Element; ///< Node representing a body element.
        } SyntacticElement;
    };

public:
    ///
    /// @brief Constructs a disjunction or conjunction constraint.
    ///
    /// @param kind Must be Disjunction or Conjunction.
    /// @param constraints The child constraints.
    /// @param isIsolated If true, solve independently of others.
    /// @param locator AST node that triggered the constraint.
    /// @param typeVars Type variables involved.
    ///
    Constraint(
        ConstraintKind kind, llvm::ArrayRef<Constraint *> constraints,
        bool isIsolated, glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    ///
    /// @brief Constructs a binary constraint between two types.
    ///
    /// @param kind The kind of constraint.
    /// @param first The first type.
    /// @param second The second type.
    /// @param locator AST node responsible.
    /// @param typeVars Type variables involved.
    ///
    Constraint(
        ConstraintKind kind, glu::gil::Type first, glu::gil::Type second,
        glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    ///
    /// @brief Constructs a ternary constraint (e.g., for overloads).
    ///
    /// @param kind Constraint kind.
    /// @param first First type.
    /// @param second Second type.
    /// @param third Third type.
    /// @param locator AST source node.
    /// @param typeVars Type variables involved.
    ///
    Constraint(
        ConstraintKind kind, glu::gil::Type first, glu::gil::Type second,
        glu::gil::Type third, glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    ///
    /// @brief Constructs a constraint with a conversion restriction.
    ///
    /// @param kind Constraint kind.
    /// @param restriction Conversion restriction kind.
    /// @param first Source type.
    /// @param second Target type.
    /// @param locator AST source node.
    /// @param typeVars Type variables involved.
    ///
    Constraint(
        ConstraintKind kind, ConversionRestrictionKind restriction,
        glu::gil::Type first, glu::gil::Type second, glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    ///
    /// @brief Gets the mutable buffer of type variables.
    ///
    /// @return Mutable array of type variables.
    ///
    llvm::MutableArrayRef<glu::types::TypeVariableTy *> getTypeVariablesBuffer()
    {
        return { getTrailingObjects<glu::types::TypeVariableTy *>(),
                 NumTypeVariables };
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINTS_HPP
