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
          Constraint, glu::types::TypeVariableTy *, glu::ast::FunctionDecl *> {

    using TrailingArgs = llvm::TrailingObjects<
        Constraint, glu::types::TypeVariableTy *, glu::ast::FunctionDecl *>;

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
    unsigned
        _hasDeclContext : 1; ///< Whether the constraint has a decl context.

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

    Constraint(
        glu::ast::ASTNode node, bool isDiscarded, glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    Constraint(
        glu::types::Ty type, glu::ast::FunctionDecl *choice,
        glu::ast::ASTNode *locator,
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

    template <typename T>
    size_t numTrailingObjects(TrailingObjects::OverloadToken<T>) const
    {
        if constexpr (std::is_same_v<T, glu::types::TypeVariableTy *>) {
            return _numTypeVariables;
        } else if constexpr (std::is_same_v<T, glu::ast::FunctionDecl *>) {
            return (_kind == ConstraintKind::BindOverload) ? 1 : 0;
        } else {
            return 0;
        }
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

    static Constraint *createSyntacticElement(
        glu::types::Ty var, llvm::BumpPtrAllocator &allocator,
        glu::ast::ASTNode node, glu::ast::ASTNode *locator, bool isDiscarded
    );

    static Constraint *createConjunction(
        llvm::BumpPtrAllocator &allocator,
        llvm::ArrayRef<Constraint *> constraints, glu::ast::ASTNode *locator,
        llvm::ArrayRef<glu::types::TypeVariableTy *> referencedVars
    );

    static Constraint *createRestricted(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        ConversionRestrictionKind restriction, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createBindOverload(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
        glu::ast::FunctionDecl *choice, glu::ast::ASTNode *locator
    );

    static Constraint *createDisjunction(
        llvm::BumpPtrAllocator &allocator,
        llvm::ArrayRef<Constraint *> constraints, glu::ast::ASTNode *locator,
        bool rememberChoice
    );

    static Constraint *createMemberOrOuterDisjunction(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        glu::types::Ty first, glu::types::Ty second,
        glu::ast::StructMemberExpr *member,
        llvm::ArrayRef<glu::ast::FunctionDecl *> outerAlternatives,
        glu::ast::ASTNode *locator
    );


    /// @brief Gets the kind of constraint.
    /// @return The kind of constraint.
    ConstraintKind getKind() const { return _kind; }

    /// @brief Gets the restriction kind.
    /// @return The conversion restriction kind.
    ConversionRestrictionKind getRestriction() const
    {
        assert(_hasRestriction && "No restriction available");
        return _restriction;
    }
    /// @brief Gets the first type involved in the constraint.
    /// @return The first type.

    glu::types::Ty getFirstType() const
    {
        assert(
            _kind != ConstraintKind::Disjunction
            && _kind != ConstraintKind::Conjunction
        );
        return _types.First;
    }

    /// @brief Gets the second type involved in the constraint.
    /// @return The second type.
    glu::types::Ty getSecondType() const
    {
        assert(
            _kind != ConstraintKind::Disjunction
            && _kind != ConstraintKind::Conjunction
        );
        return _types.Second;
    }

    /// @brief Gets the member involved in the constraint.
    /// @return The member expression.
    glu::ast::StructMemberExpr *getMember() const
    {
        assert(
            _kind == ConstraintKind::ValueMember
            || _kind == ConstraintKind::UnresolvedValueMember
        );
        return _member.structMember;
    }

    /// @brief Gets the overload involved in the constraint.
    /// @return The overload type.
    glu::types::Ty getOverload() const
    {
        assert(_kind == ConstraintKind::BindOverload);
        return _overload.First;
    }
    /// @brief Gets the locator for the constraint.
    /// @return The AST node responsible for the constraint.
    glu::ast::ASTNode *getLocator() const { return _locator; }

    /// @brief Gets the syntactic element for the constraint.
    /// @return The syntactic element.
    glu::ast::ASTNode getSyntacticElement() const
    {
        assert(_kind == ConstraintKind::SyntacticElement);
        return _syntacticElement.Element;
    }

    /// Retrieve the set of constraints in a disjunction.
    llvm::ArrayRef<Constraint *> getNestedConstraints() const
    {
        assert(
            _kind == ConstraintKind::Disjunction
            || _kind == ConstraintKind::Conjunction
        );
        return _nested;
    }

    glu::ast::FunctionDecl *getOverloadChoice() const
    {
        assert(_kind == ConstraintKind::BindOverload);
        return *getTrailingObjects<glu::ast::FunctionDecl *>();
    }

    /// @brief Gets the type variables involved in the constraint.
    /// @return The type variables.
    llvm::ArrayRef<glu::types::TypeVariableTy *> getTypeVariables() const
    {
        return { getTrailingObjects<glu::types::TypeVariableTy *>(),
                 _numTypeVariables };
    }

    void setFavored(bool isFavored) { _isFavored = isFavored; }
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINTS_HPP
