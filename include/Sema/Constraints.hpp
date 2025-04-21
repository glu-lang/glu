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
    GenericArguments, ///< Explicit generic args for overload.
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
enum class ConversionRestrictionKind : char {
    DeepEquality, ///< Deep structural equality.
    ArrayToPointer, ///< Array to pointer conversion.
    StringToPointer, ///< String to pointer conversion.
    PointerToPointer ///< Pointer to pointer conversion.
};
} // namespace glu::sema

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
      private llvm::TrailingObjects<Constraint, glu::types::TypeVariableTy *> {

    using TrailingArgs
        = llvm::TrailingObjects<Constraint, glu::types::TypeVariableTy *>;

    // Make the TrailingObjects base class a friend to allow proper access
    friend TrailingArgs;

    ConstraintKind _kind; ///< Kind of constraint.

    ConversionRestrictionKind
        _restriction; ///< Optional conversion restriction.

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
            glu::types::Ty first; ///< First type involved.
            glu::types::Ty second; ///< Second type involved.
        } _types;

        struct {
            glu::types::Ty first; ///< Base type.
            glu::types::Ty second; ///< Member type.
            glu::ast::StructMemberExpr *structMember; ///< Member node.
        } _member;

        llvm::ArrayRef<Constraint *> _nested; ///< Nested constraints.

        struct {
            glu::types::Ty first; ///< Overload target type.
            glu::ast::FunctionDecl
                *overloadChoice; ///< Function declaration for overload choice.
        } _overload;

        struct {
            glu::ast::ASTNode element; ///< Node representing a body element.
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

    /// @brief Constructs a syntactic element constraint.
    ///
    /// @param node The AST node representing the syntactic element.
    /// @param isDiscarded Whether the result of this constraint is unused.
    /// @param locator The AST node that triggered this constraint.
    /// @param typeVars Type variables involved in the constraint.
    ///
    Constraint(
        glu::ast::ASTNode node, bool isDiscarded, glu::ast::ASTNode *locator,
        llvm::SmallPtrSetImpl<glu::types::TypeVariableTy *> &typeVars
    );

    /// @brief Constructs a bind overload constraint.
    ///
    /// @param type The type to be constrained.
    /// @param choice The function declaration representing the overload choice.
    /// @param locator The AST node that triggered this constraint.
    /// @param typeVars Type variables involved in the constraint.
    ///
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

    size_t numTrailingObjects(
        TrailingObjects::OverloadToken<glu::types::TypeVariableTy *>
    ) const
    {
        return _numTypeVariables;
    }

public:
    /// @brief Create a new constraint between two types.
    /// @param allocator The allocator for memory allocation.
    /// @param kind The kind of constraint to create.
    /// @param first The first type in the constraint.
    /// @param second The second type in the constraint.
    /// @param locator The AST node that triggered this constraint.
    /// @param extraTypeVars Additional type variables to include.
    /// @return A newly created constraint.
    static Constraint *create(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator,
        llvm::ArrayRef<glu::types::TypeVariableTy *> extraTypeVars = {}
    );

    static Constraint *createBind(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createEqual(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createBindToPointerType(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createConversion(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createConversion(
        llvm::BumpPtrAllocator &allocator,
        ConversionRestrictionKind restriction, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createArgumentConversion(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createArgumentConversion(
        llvm::BumpPtrAllocator &allocator,
        ConversionRestrictionKind restriction, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createOperatorArgumentConversion(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createOperatorArgumentConversion(
        llvm::BumpPtrAllocator &allocator,
        ConversionRestrictionKind restriction, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createCheckedCast(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createCheckedCast(
        llvm::BumpPtrAllocator &allocator,
        ConversionRestrictionKind restriction, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createDefaultable(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createGenericArguments(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    static Constraint *createLValueObject(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    /// @brief Create a member constraint.
    /// @param allocator The allocator for memory allocation.
    /// @param kind The kind of constraint to create.
    /// @param first The base type.
    /// @param second The member type.
    /// @param member The struct member expression.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created member constraint.
    static Constraint *createMember(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        glu::types::Ty first, glu::types::Ty second,
        glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
    );

    /// @brief Create a syntactic element constraint.
    /// @param var The type variable.
    /// @param allocator The allocator for memory allocation.
    /// @param node The AST node representing the element.
    /// @param locator The AST node that triggered this constraint.
    /// @param isDiscarded Whether the result is unused.
    /// @return A newly created syntactic element constraint.
    static Constraint *createSyntacticElement(
        glu::types::Ty var, llvm::BumpPtrAllocator &allocator,
        glu::ast::ASTNode node, glu::ast::ASTNode *locator, bool isDiscarded
    );

    /// @brief Create a conjunction constraint (AND of multiple constraints).
    /// @param allocator The allocator for memory allocation.
    /// @param constraints The constraints that must all be satisfied.
    /// @param locator The AST node that triggered this constraint.
    /// @param referencedVars The type variables referenced by this conjunction.
    /// @return A newly created conjunction constraint.
    static Constraint *createConjunction(
        llvm::BumpPtrAllocator &allocator,
        llvm::ArrayRef<Constraint *> constraints, glu::ast::ASTNode *locator,
        llvm::ArrayRef<glu::types::TypeVariableTy *> referencedVars
    );

    /// @brief Create a constraint with a specific conversion restriction.
    /// @param allocator The allocator for memory allocation.
    /// @param kind The kind of constraint to create.
    /// @param restriction The specific conversion rule to apply.
    /// @param first The first type in the constraint.
    /// @param second The second type in the constraint.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created restricted constraint.
    static Constraint *createRestricted(
        llvm::BumpPtrAllocator &allocator, ConstraintKind kind,
        ConversionRestrictionKind restriction, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    );

    /// @brief Create a bind overload constraint.
    /// @param allocator The allocator for memory allocation.
    /// @param type The type to be constrained.
    /// @param choice The function declaration representing the overload choice.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created bind overload constraint.
    static Constraint *createBindOverload(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
        glu::ast::FunctionDecl *choice, glu::ast::ASTNode *locator
    );

    /// @brief Create a disjunction constraint (OR of multiple constraints).
    /// @param allocator The allocator for memory allocation.
    /// @param constraints The constraints where at least one must be satisfied.
    /// @param locator The AST node that triggered this constraint.
    /// @param rememberChoice Whether to remember which constraint was chosen.
    /// @return A newly created disjunction constraint.
    static Constraint *createDisjunction(
        llvm::BumpPtrAllocator &allocator,
        llvm::ArrayRef<Constraint *> constraints, glu::ast::ASTNode *locator,
        bool rememberChoice
    );

    /// @brief Create a member or outer disjunction constraint.
    /// @param allocator The allocator for memory allocation.
    /// @param kind The kind of constraint to create.
    /// @param first The first type in the constraint.
    /// @param second The second type in the constraint.
    /// @param member The struct member expression.
    /// @param outerAlternatives Alternative function declarations.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created member or outer disjunction constraint.
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
        if (_kind == ConstraintKind::BindOverload)
            return _overload.first;
        return _types.first;
    }

    /// @brief Gets the second type involved in the constraint.
    /// @return The second type.
    glu::types::Ty getSecondType() const
    {
        assert(
            _kind != ConstraintKind::Disjunction
            && _kind != ConstraintKind::Conjunction
        );
        return _types.second;
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
        return _overload.first;
    }

    /// @brief Get the Overload choice.
    /// @return The overload choice.
    /// @details This function is only valid for BindOverload constraints.
    /// @return The overload choice.
    glu::ast::FunctionDecl *getOverloadChoice() const
    {
        assert(_kind == ConstraintKind::BindOverload);
        return _overload.overloadChoice;
    }

    /// @brief Gets the locator for the constraint.
    /// @return The AST node responsible for the constraint.
    glu::ast::ASTNode *getLocator() const { return _locator; }

    /// @brief Gets the syntactic element for the constraint.
    /// @return The syntactic element.
    glu::ast::ASTNode getSyntacticElement() const
    {
        assert(_kind == ConstraintKind::SyntacticElement);
        return _syntacticElement.element;
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

    /// @brief Gets the type variables involved in the constraint.
    /// @return The type variables.
    llvm::ArrayRef<glu::types::TypeVariableTy *> getTypeVariables() const
    {
        return { getTrailingObjects<glu::types::TypeVariableTy *>(),
                 _numTypeVariables };
    }
    /// @brief Sets whether this constraint is favored.
    /// @param isFavored True if this constraint should be favored, false
    /// otherwise.
    void setFavored(bool isFavored) { _isFavored = isFavored; }

    /// @brief Checks if this constraint is currently active.
    /// @return True if the constraint is active, false otherwise.
    bool isActive() const { return _isActive; }

    /// @brief Checks if this constraint is disabled.
    /// @return True if the constraint is disabled, false otherwise.
    bool isDisabled() const { return _isDisabled; }

    /// @brief Checks if this constraint is favored.
    /// @return True if the constraint is favored, false otherwise.
    bool isFavored() const { return _isFavored; }

    /// @brief Checks if this constraint has a restriction.
    /// @return True if the constraint has a restriction, false otherwise.
    bool hasRestriction() const { return _hasRestriction; }

    /// @brief Checks if this constraint is discarded.
    /// @return True if the constraint is discarded, false otherwise.
    bool isDiscarded() const { return _isDiscarded; }

    /// @brief Checks if the choice of this disjunction should be remembered.
    /// @return True if the choice should be remembered, false otherwise.
    bool shouldRememberChoice() const { return _rememberChoice; }
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINTS_HPP
