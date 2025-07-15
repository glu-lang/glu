#ifndef GLU_GILGEN_INTRINSICS_HPP
#define GLU_GILGEN_INTRINSICS_HPP

#include "Context.hpp"
#include "GIL/Instructions.hpp"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::gilgen {

/// @brief Utility class for generating intrinsic arithmetic operations
class IntrinsicGenerator {
private:
    Context &_ctx;

public:
    IntrinsicGenerator(Context &ctx) : _ctx(ctx) { }

    /// @brief Generate intrinsic arithmetic operation for integers only
    /// @param op The operator name ("+", "-", "*", "/")
    /// @param lhs Left-hand operand
    /// @param rhs Right-hand operand
    /// @return Generated instruction value
    gil::Value
    generateArithmeticOp(llvm::StringRef op, gil::Value lhs, gil::Value rhs)
    {
        // Only handle integer arithmetic for now
        types::TypeBase *lhsType = lhs.getType().getType();
        types::TypeBase *rhsType = rhs.getType().getType();

        // Ensure we're working with integers only
        if (!llvm::isa<types::IntTy>(lhsType) || !llvm::isa<types::IntTy>(rhsType)) {
            llvm::errs() << "Error: Non-integer arithmetic not supported yet\n";
            return gil::Value();
        }

        if (op == "+") {
            return _ctx.buildAdd(lhs, rhs)->getResult(0);
        } else if (op == "-") {
            return _ctx.buildSub(lhs, rhs)->getResult(0);
        } else if (op == "*") {
            return _ctx.buildMul(lhs, rhs)->getResult(0);
        } else if (op == "/") {
            return _ctx.buildSDiv(lhs, rhs)->getResult(0);
        }

        llvm::errs() << "Error: Unknown arithmetic operator: " << op << "\n";
        return gil::Value();
        llvm::errs() << "Error: Unknown arithmetic operator: " << op << "\n";
        return gil::Value();
    }

    /// @brief Check if the given operator is an intrinsic arithmetic operator
    /// @param op The operator name
    /// @return True if it's an intrinsic arithmetic operator
    static bool isIntrinsicArithmeticOp(llvm::StringRef op)
    {
        return op == "+" || op == "-" || op == "*" || op == "/";
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_INTRINSICS_HPP
