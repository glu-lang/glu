#ifndef GLU_SEMA_CONSTRAINT_HPP
#define GLU_SEMA_CONSTRAINT_HPP

#include "AST/Exprs.hpp"
#include "AST/Types.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallPtrSet.h>
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
    CheckedCast, ///< Checked cast from first to second type.
    BindOverload, ///< Binds to a specific overload.
    ValueMember, ///< First type has a value member of second type.
    Defaultable, ///< First type can default to second.
    Disjunction, ///< One or more constraints must hold.
    Conjunction, ///< All constraints must hold.
    ExpressibleByIntLiteral, ///< Can be expressed as an integer literal.
    ExpressibleByStringLiteral, ///< Can be expressed as a string literal.
    ExpressibleByFloatLiteral, ///< Can be expressed as a float literal.
    ExpressibleByBoolLiteral, ///< Can be expressed as a boolean literal.
    NumberOfConstraints, ///< Total number of constraint kinds.
    StructInitialiser ///< Struct can be initialized with given fields.
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
/// @class Constraint
/// @brief Represents a constraint between types or variables.
///
/// Constraints express relations between type variables or concrete types
/// in the type system. They are used in type inference and checking.
///
class Constraint {

    ConstraintKind _kind; ///< Kind of constraint.

    unsigned _isDisabled : 1 = 0; ///< Whether this constraint is disabled.
    unsigned _hasSucceeded : 1 = 0; ///< Whether the constraint has succeeded.
    unsigned _hasFailed : 1 = 0; ///< Whether the constraint has failed.

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

        glu::types::Ty _singleType; ///< Single type involved.
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
        glu::ast::ASTNode *locator
    );

    ///
    /// @brief Constructs a constraint on one type.
    ///
    /// @param kind The kind of constraint.
    /// @param type The type involved.
    /// @param locator AST node responsible.
    /// @param typeVars Type variables involved.
    ///
    Constraint(
        ConstraintKind kind, glu::types::Ty type, glu::ast::ASTNode *locator
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
        glu::ast::ASTNode *locator
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
    /// @brief Constructs a constraint with a Member.
    /// @param kind Constraint kind.
    /// @param first First type.
    /// @param second Second type.
    /// @param member The member expression.
    /// @param locator AST source node.
    /// @param typeVars Type variables involved.
    Constraint(
        ConstraintKind kind, glu::types::Ty first, glu::types::Ty second,
        glu::ast::StructMemberExpr *member, glu::ast::ASTNode *locator
    );

    /// @brief Constructs a syntactic element constraint.
    ///
    /// @param node The AST node representing the syntactic element.
    /// @param isDiscarded Whether the result of this constraint is unused.
    /// @param locator The AST node that triggered this constraint.
    /// @param typeVars Type variables involved in the constraint.
    ///
    Constraint(
        glu::ast::ASTNode node, bool isDiscarded, glu::ast::ASTNode *locator
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
        glu::ast::ASTNode *locator
    );

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
        glu::types::Ty first, glu::types::Ty second, glu::ast::ASTNode *locator
    );

    /// @brief Create a bind constraint between two types.
    /// @param allocator The allocator for memory allocation.
    /// @param first The first type in the bind.
    /// @param second The second type in the bind.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created bind constraint.
    static Constraint *createBind(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    )
    {
        return create(allocator, ConstraintKind::Bind, first, second, locator);
    }

    /// @brief Create a structural equality constraint between two types.
    /// @param allocator The allocator for memory allocation.
    /// @param first The first type in the equality check.
    /// @param second The second type in the equality check.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created equality constraint.
    static Constraint *createEqual(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    )
    {
        return create(allocator, ConstraintKind::Equal, first, second, locator);
    }

    /// @brief Create a bind-to-pointer constraint.
    /// @param allocator The allocator for memory allocation.
    /// @param first The original type.
    /// @param second The expected pointer type.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created bind-to-pointer constraint.
    static Constraint *createBindToPointerType(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    )
    {
        return create(
            allocator, ConstraintKind::BindToPointerType, first, second, locator
        );
    }

    /// @brief Create a conversion constraint between two types.
    /// @param allocator The allocator for memory allocation.
    /// @param first The source type.
    /// @param second The target type.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created conversion constraint.
    static Constraint *createConversion(
        llvm::BumpPtrAllocator &allocator, glu::ast::ExprBase *node,
        types::Ty expected
    )
    {
        return create(
            allocator, ConstraintKind::Conversion, node->getType(), expected,
            node
        );
    }

    /// @brief Create a checked cast constraint between two types.
    /// @param allocator The allocator for memory allocation.
    /// @param first The source type.
    /// @param second The target type.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created checked cast constraint.
    static Constraint *createCheckedCast(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    )
    {
        return create(
            allocator, ConstraintKind::CheckedCast, first, second, locator
        );
    }

    /// @brief Create a defaultable constraint, where the second type is a
    /// fallback for the first.
    /// @param allocator The allocator for memory allocation.
    /// @param first The type that may default to the second.
    /// @param second The default type.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created defaultable constraint.
    static Constraint *createDefaultable(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty first,
        glu::types::Ty second, glu::ast::ASTNode *locator
    )
    {
        return create(
            allocator, ConstraintKind::Defaultable, first, second, locator
        );
    }

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
    )
    {
        return new (allocator) Constraint(kind, first, second, member, locator);
    }

    /// @brief Create a conjunction constraint (AND of multiple constraints).
    /// @param allocator The allocator for memory allocation.
    /// @param constraints The constraints that must all be satisfied.
    /// @param locator The AST node that triggered this constraint.
    /// @param referencedVars The type variables referenced by this conjunction.
    /// @return A newly created conjunction constraint.
    static Constraint *createConjunction(
        llvm::BumpPtrAllocator &allocator,
        llvm::ArrayRef<Constraint *> constraints, glu::ast::ASTNode *locator
    );

    /// @brief Create a constraint with a specific conversion restriction.
    /// @param allocator The allocator for memory allocation.
    /// @param kind The kind of constraint to create.
    /// @brief Create a bind overload constraint.
    /// @param allocator The allocator for memory allocation.
    /// @param type The type to be constrained.
    /// @param choice The function declaration representing the overload choice.
    /// @param locator The AST node that triggered this constraint.
    /// @return A newly created bind overload constraint.
    static Constraint *createBindOverload(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
        glu::ast::FunctionDecl *choice, glu::ast::ASTNode *locator
    )
    {
        return new (allocator) Constraint(type, choice, locator);
    }

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

    static Constraint *createExpressibleByLiteral(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
        glu::ast::ASTNode *locator, ConstraintKind kind
    );

    static Constraint *createStructInitialiser(
        llvm::BumpPtrAllocator &allocator, glu::types::Ty type,
        glu::ast::ASTNode *locator
    )
    {
        return new (allocator)
            Constraint(ConstraintKind::StructInitialiser, type, locator);
    }

    /// @brief Gets the kind of constraint.
    /// @return The kind of constraint.
    ConstraintKind getKind() const { return _kind; }

    /// @brief Checks if the constraint has succeeded.
    /// @return True if the constraint has succeeded, false otherwise.
    bool hasSucceeded() const { return _hasSucceeded; }

    /// @brief Marks the constraint as succeeded.
    void markSucceeded() { _hasSucceeded = 1; }

    /// @brief Checks if the constraint has failed.
    /// @return True if the constraint has failed, false otherwise.
    bool hasFailed() const { return _hasFailed; }

    /// @brief Marks the constraint as failed.
    void markFailed() { _hasFailed = 1; }

    bool isTypePropertyConstraint() const
    {
        return _kind == ConstraintKind::ExpressibleByIntLiteral
            || _kind == ConstraintKind::ExpressibleByStringLiteral
            || _kind == ConstraintKind::ExpressibleByFloatLiteral
            || _kind == ConstraintKind::ExpressibleByBoolLiteral;
    }

    glu::types::Ty getSingleType() const
    {
        assert(
            isTypePropertyConstraint()
            || _kind == ConstraintKind::StructInitialiser
                && "Should be a single-type constraint"
        );
        return _singleType;
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
        assert(_kind == ConstraintKind::ValueMember);
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

    /// Retrieve the set of constraints in a disjunction.
    llvm::ArrayRef<Constraint *> getNestedConstraints() const
    {
        assert(
            _kind == ConstraintKind::Disjunction
            || _kind == ConstraintKind::Conjunction
        );
        return _nested;
    }

    /// @brief Checks if this constraint is disabled.
    /// @return True if the constraint is disabled, false otherwise.
    bool isDisabled() const { return _isDisabled; }

    /// @brief Disable this constraint.
    void disable() { _isDisabled = 1; }
    /// @brief Enable this constraint.
    void enable() { _isDisabled = 0; }

    /// @brief Sets the enabled state of this constraint.
    /// @param enabled True to enable the constraint, false to disable it.
    void setEnabled(bool enabled) { _isDisabled = !enabled; }

    /// @brief Print this constraint to the output stream.
    void print() const;
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINT_HPP
