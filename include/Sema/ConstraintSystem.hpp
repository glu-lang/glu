#ifndef GLU_SEMA_CONSTRAINT_SYSTEM_HPP
#define GLU_SEMA_CONSTRAINT_SYSTEM_HPP

#include "Basic/Diagnostic.hpp"
#include "Constraint.hpp"
#include "ScopeTable.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/ilist.h>

namespace glu::sema {

using Score = unsigned;

/// @brief Represents a solution to a set of constraints.
struct Solution {
    /// @brief Inferred types for expressions (expr -> type).
    llvm::DenseMap<glu::ast::ExprBase *, glu::types::TypeBase *> exprTypes;

    /// @brief Type variable bindings (type variable -> type).
    llvm::DenseMap<glu::types::TypeVariableTy *, glu::types::TypeBase *>
        typeBindings;

    /// @brief Overload choices made (expr -> function declaration).
    llvm::DenseMap<glu::ast::RefExpr *, glu::ast::FunctionDecl *>
        overloadChoices;

    /// @brief Implicit conversions applied to expressions (expr -> target
    /// type).
    llvm::DenseMap<glu::ast::ExprBase *, types::TypeBase *> implicitConversions;

    /// @brief Retrieves the type of an expression (after resolution).
    /// @param expr The expression to query.
    /// @return The inferred type, or nullptr if not found.
    glu::types::TypeBase *getTypeFor(glu::ast::ExprBase *expr) const
    {
        auto it = exprTypes.find(expr);
        return (it != exprTypes.end()) ? it->second : nullptr;
    }

    glu::types::TypeBase *getTypeFor(glu::types::TypeVariableTy *var)
    {
        auto it = typeBindings.find(var);
        return (it != typeBindings.end()) ? it->second : nullptr;
    }

    /// @brief Records an inferred type for a given expression.
    /// @param expr The expression.
    /// @param type The inferred type.
    void recordExprType(glu::ast::ExprBase *expr, glu::types::TypeBase *type)
    {
        exprTypes[expr] = type;
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
    /// @brief Inferred types for expressions (expr -> type).
    llvm::DenseMap<glu::ast::ExprBase *, glu::types::TypeBase *> exprTypes;

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

/// @brief Manages type constraints and their resolution in the current context.
class ConstraintSystem {
    ScopeTable *_scopeTable; ///< The scope table for the current context.
    std::vector<glu::types::TypeVariableTy *>
        _typeVariables; ///< List of type variables.
    llvm::BumpPtrAllocator _allocator; ///< Allocator for memory management.
    std::vector<Constraint *>
        _constraints; ///< List of constraints to be solved.
    llvm::DenseMap<Constraint *, std::pair<unsigned, Constraint *>>
        _bestSolutions; ///< Best solution per disjunction and its score.
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

    /// @brief Gets the best solution found for a given constraint.
    /// @param constraint The constraint for which to retrieve the solution.
    /// @return The best solution constraint, or nullptr if none.
    Constraint *getBestSolution(Constraint *constraint);

    /// @brief Gets the score of the best solution for a constraint.
    /// @param constraint The constraint to query.
    /// @return The score associated with the best solution.
    unsigned getBestSolutionScore(Constraint *constraint);

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

    /// @brief Registers the best solution and score for a constraint.
    /// @param constraint The original constraint.
    /// @param solution The best constraint solution.
    /// @param score The associated score.
    void setBestSolution(
        Constraint *constraint, Constraint *solution, unsigned score
    )
    {
        _bestSolutions[constraint] = std::make_pair(score, solution);
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
    /// @return True if the constraint was successfully applied, false
    /// otherwise.
    bool apply(
        Constraint *constraint, SystemState &state,
        std::vector<SystemState> &worklist
    );

    void mapTypeVariables(SolutionResult &solutionRes);
    /// @brief Tries to apply a binding constraint.
    /// @param constraint The binding constraint to apply.
    /// @param state The current system state.
    /// @return True if the binding was successful, false otherwise.
    bool applyBind(Constraint *constraint, SystemState &state);

    /// @brief Solves all constraints currently stored in the system.
    ///
    /// This function initializes a worklist with a base system state and
    /// explores all possible resolution paths by applying constraints
    /// iteratively. For disjunctive or ambiguous constraints, the worklist may
    /// branch by cloning the current state. Valid and complete states are
    /// converted into solutions, and the best (lowest-score) ones are retained.
    void solveConstraints();

    /// @brief Applies a defaultable constraint in the current state.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return True if the constraint was successfully applied, false
    /// otherwise.
    bool applyDefaultable(
        Constraint *constraint, SystemState &state,
        std::vector<SystemState> &worklist
    );
};

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINT_SYSTEM_HPP
