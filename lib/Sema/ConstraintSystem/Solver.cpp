#include "ConstraintSystem.hpp"

namespace glu::sema {

/// @brief Helper function to count conversions for function-like expressions
/// (calls, operators)
/// @param functionTy The function type of the operator/callee
/// @param concreteTy The concrete function type we're comparing against
/// @param expr The call, binary, or unary expression
/// @param state The SystemState to use for recursive conversion counting
/// @return The total number of conversions needed
static size_t countFunctionConversions(
    types::FunctionTy *functionTy, types::FunctionTy *concreteTy,
    ast::ExprBase *expr, SystemState const *state
)
{
    size_t count = 0;

    // Handle return type - all expressions return their result type
    count += state->getExprConversionCount(expr, concreteTy->getReturnType());

    // Handle operands/arguments based on expression type
    if (auto *callExpr = llvm::dyn_cast<ast::CallExpr>(expr)) {
        // Function call: handle parameters (don't touch variadic parameters)
        for (std::size_t i = 0; i < functionTy->getParameterCount()
             && i < callExpr->getArgs().size();
             ++i) {
            auto *paramTy = functionTy->getParameter(i);
            count += state->getExprConversionCount(
                callExpr->getArgs()[i], paramTy
            );
        }
    } else if (auto *binaryOp = llvm::dyn_cast<ast::BinaryOpExpr>(expr)) {
        // Binary operators have two operands
        if (functionTy->getParameterCount() >= 2) {
            count += state->getExprConversionCount(
                binaryOp->getLeftOperand(), functionTy->getParameter(0)
            );
            count += state->getExprConversionCount(
                binaryOp->getRightOperand(), functionTy->getParameter(1)
            );
        }
    } else if (auto *unaryOp = llvm::dyn_cast<ast::UnaryOpExpr>(expr)) {
        // Unary operators have one operand
        if (functionTy->getParameterCount() >= 1) {
            count += state->getExprConversionCount(
                unaryOp->getOperand(), functionTy->getParameter(0)
            );
        }
    }

    return count;
}

size_t SystemState::getExprConversionCount(
    glu::ast::ExprBase *expr, types::TypeBase *targetType
) const
{
    auto *substitutedExprType
        = substitute(expr->getType(), typeBindings, _context);
    if (substitutedExprType == targetType) {
        return 0; // No conversion needed
    }

    // Special handling for function calls, binary operators, and unary
    // operators: need to consider return type and parameter types
    auto *functionTy = getUnderlyingFunctionType(substitutedExprType);
    auto *concreteTy = getUnderlyingFunctionType(targetType);
    if (!functionTy || !concreteTy)
        return 1;

    // Check if this RefExpr is used as a function callee
    auto *callExpr = llvm::dyn_cast<ast::CallExpr>(expr->getParent());
    if (callExpr && callExpr->getCallee() == expr) {
        return countFunctionConversions(functionTy, concreteTy, callExpr, this);
    }

    // Check if this RefExpr is used as a binary operator
    auto *binaryOpExpr = llvm::dyn_cast<ast::BinaryOpExpr>(expr->getParent());
    if (binaryOpExpr && binaryOpExpr->getOperator() == expr) {
        return countFunctionConversions(
            functionTy, concreteTy, binaryOpExpr, this
        );
    }

    // Check if this RefExpr is used as a unary operator
    auto *unaryOpExpr = llvm::dyn_cast<ast::UnaryOpExpr>(expr->getParent());
    if (unaryOpExpr && unaryOpExpr->getOperator() == expr) {
        return countFunctionConversions(
            functionTy, concreteTy, unaryOpExpr, this
        );
    }

    return 1;
}

size_t SystemState::getImplicitConversionCount() const
{
    // At this point, all implicit conversions have been recorded in the map.
    // Some may have been substituted and are no longer needed, they should not
    // be counted. Additionally, conversions for function calls may need to be
    // counted as multiple conversions.
    size_t count = 0;
    for (auto const &[expr, type] : implicitConversions) {
        // substitute the type to see if it's still needed
        auto substitutedType = substitute(type, typeBindings, _context);
        count += getExprConversionCount(expr, substitutedType);
    }
    return count;
}

std::weak_ordering SystemState::operator<=>(SystemState const &other) const
{
    // less conversions is better
    auto compareConversions = this->getImplicitConversionCount()
        <=> other.getImplicitConversionCount();
    if (compareConversions != 0)
        return compareConversions;
    // more satisfied defaultable constraints is better
    auto compareDefaultables = other.defaultableConstraintsSatisfied
        <=> this->defaultableConstraintsSatisfied;
    if (compareDefaultables != 0)
        return compareDefaultables;
    return std::weak_ordering::equivalent;
}

void SolutionResult::tryAddSolution(SystemState const &s)
{
    // If no previous solutions exist just add directly
    if (solutions.empty()) {
        solutions.push_back(std::move(s));
        return;
    }

    auto comparison = s <=> solutions.front();
    if (comparison == std::weak_ordering::less) {
        // if there is a better solution then replace previous ones
        // (less is better here since it means fewer conversions)
        solutions.clear();
        solutions.push_back(std::move(s));
    } else if (comparison == std::weak_ordering::equivalent) {
        // Ambiguity: multiple equally good solutions
        solutions.push_back(std::move(s));
    } else {
        // Worse solution, ignore
        return;
    }
}

void SystemState::mergeInto(SystemState &other) const
{
    // Merge type bindings
    for (auto const &[var, type] : typeBindings) {
        other.typeBindings[var] = type;
    }

    // Merge overload choices
    for (auto const &[expr, decl] : overloadChoices) {
        other.overloadChoices[expr] = decl;
    }

    // Merge implicit conversions
    for (auto const &[expr, targetType] : implicitConversions) {
        other.implicitConversions[expr] = targetType;
    }
}

} // namespace glu::sema
