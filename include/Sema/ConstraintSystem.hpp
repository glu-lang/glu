#ifndef GLU_SEMA_CONSTRAINT_SYSTEM_HPP
#define GLU_SEMA_CONSTRAINT_SYSTEM_HPP

#include "Basic/Diagnostic.hpp"
#include "Constraint.hpp"
#include "ScopeTable.hpp"
#include "TyMapperVisitor.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/ilist.h>

namespace glu::sema {

// Forward declarations
class ConstraintSystem;
struct SystemState;
class ConversionVisitor;

/// @brief Result of applying a constraint to a system state.
enum class ConstraintResult {
    /// @brief The constraint failed to apply (incompatible types, etc.).
    Failed,
    /// @brief The constraint is already satisfied in the current state.
    Satisfied,
    /// @brief The constraint was successfully applied and may have modified the
    /// state.
    Applied
};

using Score = unsigned;

/// @brief Represents a solution to a set of constraints.
struct Solution {
    /// @brief Type variable bindings (type variable -> type).
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *>
        typeBindings;

    /// @brief Overload choices made (expr -> function declaration).
    llvm::DenseMap<glu::ast::RefExpr *, glu::ast::FunctionDecl *>
        overloadChoices;

    /// @brief Implicit conversions applied to expressions (expr -> target
    /// type).
    llvm::DenseMap<glu::ast::ExprBase *, types::TypeBase *> implicitConversions;

    glu::types::TypeBase *getTypeFor(glu::types::TypeVariableTy *var)
    {
        auto it = typeBindings.find(var);
        return (it != typeBindings.end()) ? it->second : nullptr;
    }

    /// @brief Binds a type variable to a specific type.
    /// @param var The type variable.
    /// @param type The type it is bound to.
    void
    bindTypeVar(glu::types::TypeVariableTy *var, glu::types::TypeBase *type)
    {
        typeBindings[var] = type;
    }

    /// @brief Records an overload resolution for a given expression.
    /// @param expr The expression.
    /// @param choice The selected function declaration.
    void recordOverload(glu::ast::RefExpr *expr, glu::ast::FunctionDecl *choice)
    {
        overloadChoices[expr] = choice;
    }
    /// @brief Records an implicit conversion for a given expression.
    /// @param expr The expression.
    /// @param targetType The target type of the conversion.
    void recordImplicitConversion(
        glu::ast::ExprBase *expr, types::TypeBase *targetType
    )
    {
        implicitConversions[expr] = targetType;
    }

    /// @brief Retrieves the conversion applied to an expression.
    /// @param expr The expression to query.
    /// @return The target type of the implicit conversion, or nullptr if not
    /// found.
    types::TypeBase *getImplicitConversionFor(glu::ast::ExprBase *expr) const
    {
        auto it = implicitConversions.find(expr);
        return (it != implicitConversions.end()) ? it->second : nullptr;
    }
};

/// @brief Represents a temporary state of the constraint solver during
/// exploration.
///
/// This structure holds partial or complete information about type inference,
/// overload resolution, and implicit conversions at a given point in time.
/// It is used to explore multiple resolution paths during constraint solving
/// (e.g., disjunctions, overloads, conversions).
struct SystemState {
    /// @brief Type variable bindings (type variable -> type).
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *>
        typeBindings;

    /// @brief Overload choices made for expressions (expr -> function
    /// declaration).
    llvm::DenseMap<glu::ast::RefExpr *, glu::ast::FunctionDecl *>
        overloadChoices;

    /// @brief Implicit conversions applied to expressions (expr -> converted
    /// type).
    llvm::DenseMap<glu::ast::ExprBase *, glu::types::TypeBase *>
        implicitConversions;

    /// @brief Accumulated penalty score for this state (used to compare
    /// solutions).
    Score score = 0;

    /// @brief Converts the current system state into a complete solution.
    /// @return A fully constructed solution from this state.
    Solution toSolution() const;

    /// @brief Creates a copy of this state for branching during resolution.
    /// @return A deep copy of the current state.
    SystemState clone() const { return *this; }

    /// @brief Checks whether all constraints have been resolved.
    /// @return True if no remaining constraints are pending, false otherwise.
    bool isFullyResolved(std::vector<Constraint *> const &constraints) const;
};

/// @brief Represents the result of solving a set of constraints.
struct SolutionResult {
    /// @brief All valid solutions found.
    llvm::SmallVector<Solution, 4> solutions;

    /// @brief The best solution found (the one with the lowest score).
    Score bestScore = 0;

    /// @brief Checks whether any solutions were found.
    /// @return True if at least one solution exists, false otherwise.
    bool hasSolutions() const { return !solutions.empty(); }

    /// @brief Checks whether the result is ambiguous (i.e., multiple valid
    /// solutions exist).
    /// @return returns true if there are multiple solutions.
    bool isAmbiguous() const { return solutions.size() > 1; }

    /// @brief Tries to add a new solution, checking for ambiguity and scoring.
    /// @param state The system state to convert and add as a solution.
    void tryAddSolution(SystemState const &state);

    Solution *getBestSolution()
    {
        if (hasSolutions())
            return &solutions.front();
        return nullptr;
    }
};

/// @brief Substitutes type variables with their bindings in a type.
/// @param type The type to substitute.
/// @param bindings The current type variable bindings.
/// @param context The AST context to create new types if needed.
/// @return The type with substitutions applied.
glu::types::Ty substitute(
    glu::types::Ty type,
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *> const
        &bindings,
    glu::ast::ASTContext *context
);

/// @brief Manages type constraints and their resolution in the current context.
class ConstraintSystem {
    // Allow visitor classes to access private methods
    friend class SubstitutionMapper;
    friend class OccursCheckVisitor;
    friend class UnificationVisitor;
    friend class ConversionVisitor;
    ScopeTable *_scopeTable; ///< The scope table for the current context.
    std::vector<glu::types::TypeVariableTy *>
        _typeVariables; ///< List of type variables.
    llvm::BumpPtrAllocator _allocator; ///< Allocator for memory management.
    std::vector<Constraint *>
        _constraints; ///< List of constraints to be solved.
    glu::DiagnosticManager
        &_diagManager; ///< Diagnostic manager for error reporting.
    glu::ast::ASTContext
        *_context; ///< AST context to create new types after resolution.

public:
    /// @brief Constructs a ConstraintSystem.
    /// @param scopeTable The scope table for the current context.
    ConstraintSystem(
        ScopeTable *scopeTable, glu::DiagnosticManager &diagManager,
        glu::ast::ASTContext *context
    );

    /// @brief Destroys the ConstraintSystem.
    ~ConstraintSystem() = default;

    /// @brief Gets the memory allocator.
    /// @return A reference to the allocator.
    llvm::BumpPtrAllocator &getAllocator() { return _allocator; }

    /// @brief Gets the scope table.
    /// @return The current scope table.
    ScopeTable *getScopeTable() { return _scopeTable; }

    /// @brief Gets the list of constraints.
    /// @return A reference to the vector of constraints.
    std::vector<Constraint *> &getConstraints() { return _constraints; }

    /// @brief Gets the list of type variables.
    /// @return A reference to the list of type variables.
    std::vector<glu::types::TypeVariableTy *> &getTypeVariables()
    {
        return _typeVariables;
    }

    /// @brief Adds a new type variable to the system.
    /// @param typeVar The type variable to add.
    void addTypeVariable(glu::types::TypeVariableTy *typeVar)
    {
        _typeVariables.push_back(typeVar);
    }

    /// @brief Adds a new constraint to the system.
    /// @param constraint The constraint to add.
    void addConstraint(Constraint *constraint)
    {
        _constraints.push_back(constraint);
    }

    /// @brief Applies a given constraint to the current state.
    ///
    /// This function dispatches the application logic based on the kind of
    /// constraint. It modifies the given state and may branch the resolution
    /// into multiple states via the worklist.
    ///
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @param worklist The global worklist for exploration.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult apply(
        Constraint *constraint, SystemState &state,
        std::vector<SystemState> &worklist
    );

    void mapTypeVariables(Solution *solutionRes);

    /// @brief Maps overload choices from the given solution to the AST nodes.
    /// @param solution The solution containing resolved overloads.
    void mapOverloadChoices(Solution *solution);

    /// @brief Maps implicit conversions found during constraint solving.
    /// @param solution The solution from which implicit conversions are
    /// extracted.
    void mapImplicitConversions(Solution *solution);

    /// @brief Solves all constraints and applies type mappings to the specified
    /// expressions.
    ///
    /// This method combines constraint solving with type mapping for
    /// expressions. For module expressions (part of the AST tree), type
    /// mappings are applied automatically. For standalone expressions, they are
    /// explicitly updated.
    ///
    /// @param expressions The expressions to update with inferred types.
    /// @return True if constraint solving succeeded and types were applied.
    bool
    solveConstraints(llvm::ArrayRef<glu::ast::ExprBase *> expressions = {});

    /// @brief Applies a defaultable constraint in the current state.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyDefaultable(Constraint *constraint, SystemState &state);

    /// @brief Applies a bind-to-pointer-type constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyBindToPointerType(Constraint *constraint, SystemState &state);

    /// @brief Applies a conversion constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyConversion(Constraint *constraint, SystemState &state);

    /// @brief Applies a checked cast constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyCheckedCast(Constraint *constraint, SystemState &state);

    /// @brief Applies a bind overload constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyBindOverload(Constraint *constraint, SystemState &state);

    /// @brief Applies an l-value object constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyLValueObject(Constraint *constraint, SystemState &state);

    /// @brief Applies a value member constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyValueMember(Constraint *constraint, SystemState &state);

    /// @brief Applies an unresolved value member constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyUnresolvedValueMember(Constraint *constraint, SystemState &state);

    /// @brief Applies a generic arguments constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyGenericArguments(Constraint *constraint, SystemState &state);

    /// @brief Applies a disjunction constraint (OR of multiple constraints).
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @param worklist A list of system states used to explore resolution
    /// paths.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult applyDisjunction(
        Constraint *constraint, SystemState &state,
        std::vector<SystemState> &worklist
    );

    /// @brief Applies a conjunction constraint (AND of multiple constraints).
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @param worklist A list of system states used to explore resolution
    /// paths.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult applyConjunction(
        Constraint *constraint, SystemState &state,
        std::vector<SystemState> &worklist
    );

    /// @brief Applies an expressible by int literal constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyExpressibleByIntLiteral(Constraint *constraint);

    ConstraintResult
    applyExpressibleByFloatLiteral(Constraint *constraint);

    ConstraintResult
    applyExpressibleByStringLiteral(Constraint *constraint);

    ConstraintResult
    applyExpressibleByBoolLiteral(Constraint *constraint);

    /// @brief Checks if a conversion from one type to another is valid.
    /// @param fromType The source type.
    /// @param toType The target type.
    /// @param state The current system state.
    /// @param isExplicit Whether this is an explicit conversion (checked cast).
    /// @return True if the conversion is valid.
    bool isValidConversion(
        glu::types::Ty fromType, glu::types::Ty toType, SystemState &state,
        bool isExplicit
    );

private:
    /// @brief Applies type variable mappings to module expressions.
    /// @param solutionRes The solution result containing type mappings.
    void mapTypeVariables(SolutionResult &solutionRes);

    /// @brief Directly applies type variable mappings from a solution to a list
    /// of expressions.
    /// @param solution The solution containing type variable bindings.
    /// @param expressions The expressions to update.
    void mapTypeVariablesToExpressions(
        Solution *solution, llvm::ArrayRef<glu::ast::ExprBase *> expressions
    );

    /// @brief Tries to apply a binding constraint.
    /// @param constraint The binding constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult applyBind(Constraint *constraint, SystemState &state);

    /// @brief Performs occurs check to prevent infinite types.
    /// @param var The type variable to check.
    /// @param type The type to check against.
    /// @param bindings Current type variable bindings.
    /// @return True if var occurs in type (indicating an infinite type).
    bool occursCheck(
        glu::types::TypeVariableTy *var, glu::types::Ty type,
        llvm::DenseMap<
            glu::types::TypeVariableTy *, glu::types::TypeBase *> const
            &bindings
    );

    /// @brief Attempts to unify two types.
    /// @param first The first type.
    /// @param second The second type.
    /// @param state The current system state to modify.
    /// @return True if unification succeeded.
    bool unify(glu::types::Ty first, glu::types::Ty second, SystemState &state);
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINT_SYSTEM_HPP
