#ifndef GLU_SEMA_CONSTRAINT_SYSTEM_HPP
#define GLU_SEMA_CONSTRAINT_SYSTEM_HPP

#include "AST/Types/TypeUtils.hpp"
#include "Basic/Diagnostic.hpp"
#include "Constraint.hpp"
#include "ScopeTable.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/ilist.h>

namespace glu::sema {

// Forward declarations
class ConstraintSystem;
struct SystemState;
class ConversionVisitor;

/// @brief Result of applying a constraint to a system state.
enum class ConstraintResult {
    /// @brief The constraint failed to apply (incompatible types, etc.). An
    /// error should be reported, or this overload is not valid.
    Failed,
    /// @brief The constraint is already satisfied in the current state. Nothing
    /// was changed.
    Satisfied,
    /// @brief The constraint was successfully applied and may have modified the
    /// state, but does not need to be re-evaluated.
    Applied
};

using Score = unsigned;

/// @brief Represents a temporary state of the constraint solver during
/// exploration.
///
/// This structure holds partial or complete information about type inference,
/// overload resolution, and implicit conversions at a given point in time.
/// It is used to explore multiple resolution paths during constraint solving
/// (e.g., disjunctions, overloads, conversions).
struct SystemState {
    /// @brief The AST context for creating new types.
    ast::ASTContext *_context;
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

    size_t defaultableConstraintsSatisfied = 0;

    /// @brief Creates a copy of this state for branching during resolution.
    /// @return A deep copy of the current state.
    SystemState clone() const { return *this; }

    /// @brief Merges this state into another, combining bindings and choices.
    /// @param other The target state to merge into.
    void mergeInto(SystemState &other) const;

    /// @brief Calculates the score of the current state based on implicit
    /// conversions. Less conversions yield a better score.
    /// @return The score representing the number of implicit conversions.
    size_t getImplicitConversionCount() const;

    /// @brief Compares two solution states for score-based ordering.
    /// @param other The other state to compare with.
    /// @return A weak ordering result based on the score of the states.
    std::weak_ordering operator<=>(SystemState const &other) const;

    /// @brief Gets the number of conversions needed for a given expression to
    /// reach a target type.
    /// @param expr The expression to check.
    /// @param targetType The target type to convert to.
    /// @return The number of conversions needed.
    size_t getExprConversionCount(
        glu::ast::ExprBase *expr, types::TypeBase *targetType
    ) const;
};

using Solution = SystemState;

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
    ast::ASTNode *_root; ///< The root AST node for replacing types.
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

    ast::ASTNode *getRoot() { return _root; }
    void setRoot(ast::ASTNode *node) { _root = node; }

    /// @brief Gets the list of constraints.
    /// @return A reference to the vector of constraints.
    std::vector<Constraint *> &getConstraints() { return _constraints; }

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

    /// @brief Solves all enabled constraints within this local constraint
    /// system and returns the solution result through the provided parameter.
    /// This method is called within solveConstraints, after simplification and
    /// splitting of the constraint system, and before mapping types back to the
    /// AST.
    /// @param result The solution result to populate with found solutions.
    /// @param initialState The initial state with early unification bindings.
    /// @return True if a solution was found, false otherwise.
    bool solveLocalConstraints(
        SolutionResult &result, SystemState const &initialState
    );

    /// @brief Simplifies the constraint system before solving.
    ///
    /// This method performs various optimizations on the constraint set:
    /// - Eliminates redundant constraints
    /// - Pre-filters impossible overload choices
    /// - Reorders constraints for optimal evaluation
    /// - Performs early unification where possible
    ///
    /// @return Initial SystemState with early unification bindings applied.
    SystemState simplifyConstraints();

    /// @brief Solves all constraints and applies mappings.
    ///
    /// This method combines constraint solving with type mapping for
    /// expressions. For module expressions (part of the AST tree), type
    /// mappings are applied automatically, from the root node downwards.
    ///
    /// @return True if constraint solving succeeded and types were applied.
    bool solveConstraints();

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

    /// @brief Applies a value member constraint.
    /// @param constraint The constraint to apply.
    /// @param state The current system state.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyValueMember(Constraint *constraint, SystemState &state);

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
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyExpressibleByIntLiteral(Constraint *constraint, SystemState &state);

    /// @brief Applies an expressible by float literal constraint.
    /// @param constraint The constraint to apply.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyExpressibleByFloatLiteral(Constraint *constraint, SystemState &state);

    /// @brief Applies an expressible by string literal constraint.
    /// @param constraint The constraint to apply.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyExpressibleByStringLiteral(Constraint *constraint, SystemState &state);

    /// @brief Applies an expressible by boolean literal constraint.
    /// @param constraint The constraint to apply.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyExpressibleByBoolLiteral(Constraint *constraint, SystemState &state);

    /// @brief Applies a struct initialiser constraint.
    /// @param constraint The constraint to apply.
    /// @return ConstraintResult indicating if the constraint failed, was
    /// already satisfied, or was applied.
    ConstraintResult
    applyStructInitialiser(Constraint *constraint, SystemState &state);

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
    /// @brief Marks a constraint as succeeded or failed.
    /// @param result The result of applying the constraint.
    /// @param constraint The constraint to mark.
    /// @return The same ConstraintResult passed in.
    void markConstraint(ConstraintResult result, Constraint *constraint);

    /// @brief Applies type variable mappings to module expressions.
    /// @param solutionRes The solution result containing type mappings.
    void mapTypeVariables(SolutionResult &solutionRes);

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

    /// @brief Reports a detailed error when multiple ambiguous solutions are
    /// found.
    /// @param result The solution result containing multiple solutions.
    void reportAmbiguousSolutionError(SolutionResult const &result);

    /// @brief Reports a detailed error when no solution can be found.
    void reportNoSolutionError();

    /// @brief Gets a descriptive string for a type, providing context when
    /// possible.
    /// @param type The type to describe.
    /// @param locator The AST node that caused this constraint (for context).
    /// @return A descriptive string for the type.
    std::string getTypeDescription(glu::types::TypeBase *type);

    /// @brief Gets context information about a conversion failure.
    /// @param kind The kind of conversion constraint that failed.
    /// @param locator The AST node that caused this constraint.
    /// @return A contextual message about the conversion failure.
    std::string
    getConversionContext(ConstraintKind kind, glu::ast::ASTNode *locator);

    /// @brief Extracts the literal value from an AST node if possible.
    /// @param locator The AST node that might contain a literal.
    /// @return The literal value as a string, or empty string if not a literal.
    std::string getLiteralValue(glu::ast::ASTNode *locator);

    /// @brief Shows available function overloads for a failed function call.
    /// @param functionName The name of the function being called.
    /// @param callLocation The location of the failed call.
    void showAvailableOverloads(
        glu::ast::NamespaceIdentifier const &functionIdentifier
    );

    /// @brief Print all constraints in a ConstraintSystem for debugging.
    void print();

private:
    // Constraint simplification passes
    void reorderConstraintsByPriority();
};

/// @brief Print all constraints in a ConstraintSystem for debugging.
/// @param system The constraint system to print.
/// @param os The output stream to print to (defaults to stdout).
void printConstraints(
    ConstraintSystem &system, llvm::raw_ostream &os = llvm::outs()
);

void collectTypeVariables(
    Constraint *constraint,
    llvm::DenseSet<glu::types::TypeVariableTy *> &typeVars
);

} // namespace glu::sema

#endif // GLU_SEMA_CONSTRAINT_SYSTEM_HPP
