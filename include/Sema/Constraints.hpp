#ifndef GLU_SEMA_CONSTRAINTS_HPP
#define GLU_SEMA_CONSTRAINTS_HPP

#include "AST/Exprs.hpp"
#include "GIL/Type.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::sema {

class ConstraintSystem;

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
    LValueObject, ///< First is l-value, second is object type.

    /// Represents an AST node contained in a body of a
    /// function/closure.
    /// It only has an AST node to generate constraints and
    /// infer the type for.
    SyntacticElement
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
} // namespace glu::sema

namespace glu::types {
using Ty = glu::types::TypeBase *;
}

namespace glu::sema {
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
          Constraint, glu::types::TypeVariableTy *, glu::ast::DeclBase *> {

    using TrailingArgs = llvm::TrailingObjects<
        Constraint, glu::types::TypeVariableTy *, glu::ast::DeclBase *>;

    // Make the TrailingObjects base class a friend to allow proper access
    friend TrailingArgs;

    ConstraintKind _kind; ///< Kind of constraint.

    ConversionRestrictionKind
        _restriction : 8; ///< Optional conversion restriction.

    unsigned _numTypeVariables : 11; ///< Number of type variables involved.
    unsigned _hasFix : 1; ///< Whether a fix is allocated.
    unsigned _hasRestriction : 1; ///< Whether Restriction is valid.
    unsigned _isActive : 1; ///< Is this constraint currently active.
    unsigned _isDisabled : 1; ///< Whether this constraint is disabled.
    unsigned _rememberChoice : 1; ///< Should solver record disjunction choice.
    unsigned _isFavored : 1; ///< Preferred constraint for disjunctions.
    unsigned _isDiscarded : 1; ///< Whether the result is unused.

    union {
        struct {
            glu::types::Ty First; ///< First type involved.
            glu::types::Ty Second; ///< Second type involved.
        } _types;

        struct {
            glu::types::Ty First; ///< Base type.
            glu::types::Ty Second; ///< Member type.
            glu::ast::StructMemberExpr *structMember; ///< Member node.
        } _member;

        llvm::ArrayRef<Constraint *> _nested; ///< Nested constraints.

        struct {
            glu::types::Ty First; ///< Overload target type.
        } _overload;

        struct {
            glu::ast::ASTNode Element; ///< Node representing a body element.
        } _syntacticElement;
    };

    glu::ast::ASTNode
        *_locator; ///< Where in the AST this constraint comes from.

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
        glu::ast::ASTNode *locator,
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
        ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
        glu::ast::ASTNode *locator,
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
        glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    /// @brief Constructs a constraint with a Member.
    /// @param kind Constraint kind.
    /// @param first First type.
    /// @param second Second type.
    /// @param member The member expression.
    /// @param locator AST source node.
    /// @param typeVars Type variables involved.
    Constraint(
        ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
        glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator,
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
                 _numTypeVariables };
    }

public:
    /// Create a new constraint.
    static Constraint *create(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator,
        llvm::ArrayRef<glu::types::TypeVariableTy *> extraTypeVars = {}
    );

    static Constraint *createMember(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        glu::types::Ty first, glu::types::Ty second,
        glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
    );
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINTS_HPP
