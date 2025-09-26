#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"

#include <llvm/Support/raw_ostream.h>

namespace glu::sema {

///
/// @class ConstraintPrinter
/// @brief Utility class for printing all constraints in a ConstraintSystem.
///
class ConstraintPrinter {
public:
    /// @brief Print all constraints in a ConstraintSystem.
    /// @param system The constraint system to print.
    /// @param os The output stream to print to (defaults to stdout).
    static void
    print(ConstraintSystem &system, llvm::raw_ostream &os = llvm::outs());

private:
    /// @brief Print the constraint kind as a string.
    /// @param kind The constraint kind to print.
    /// @param os The output stream to print to.
    static void printConstraintKind(ConstraintKind kind, llvm::raw_ostream &os);

    /// @brief Print indentation spaces.
    /// @param os The output stream to print to.
    /// @param indent The number of indent levels.
    static void printIndent(llvm::raw_ostream &os, unsigned indent);

    /// @brief Recursively print a constraint and its nested constraints.
    /// @param constraint The constraint to print.
    /// @param os The output stream to print to.
    /// @param indent The indentation level for formatting.
    static void printConstraintRecursive(
        Constraint const *constraint, llvm::raw_ostream &os, unsigned indent
    );
};

void Constraint::print(llvm::raw_ostream &os, unsigned indent) const
{
    // Print indentation
    for (unsigned i = 0; i < indent; ++i) {
        os << "  ";
    }

    switch (getKind()) {
    case ConstraintKind::Bind:
        // TODO: Implement printing for Bind constraint
        os << "Bind: ";
        break;

    case ConstraintKind::Equal:
        // TODO: Implement printing for Equal constraint
        os << "Equal: ";
        break;

    case ConstraintKind::BindToPointerType:
        // TODO: Implement printing for BindToPointerType constraint
        os << "BindToPointerType: ";
        break;

    case ConstraintKind::Conversion:
        // TODO: Implement printing for Conversion constraint
        os << "Conversion: ";
        break;

    case ConstraintKind::ArgumentConversion:
        // TODO: Implement printing for ArgumentConversion constraint
        os << "ArgumentConversion: ";
        break;

    case ConstraintKind::OperatorArgumentConversion:
        // TODO: Implement printing for OperatorArgumentConversion constraint
        os << "OperatorArgumentConversion: ";
        break;

    case ConstraintKind::CheckedCast:
        // TODO: Implement printing for CheckedCast constraint
        os << "CheckedCast: ";
        break;

    case ConstraintKind::BindOverload:
        // TODO: Implement printing for BindOverload constraint
        os << "BindOverload: ";
        break;

    case ConstraintKind::ValueMember:
        // TODO: Implement printing for ValueMember constraint
        os << "ValueMember: ";
        break;

    case ConstraintKind::UnresolvedValueMember:
        // TODO: Implement printing for UnresolvedValueMember constraint
        os << "UnresolvedValueMember: ";
        break;

    case ConstraintKind::Defaultable:
        // TODO: Implement printing for Defaultable constraint
        os << "Defaultable: ";
        break;

    case ConstraintKind::Disjunction:
        // TODO: Implement printing for Disjunction constraint
        os << "Disjunction: ";
        break;

    case ConstraintKind::Conjunction:
        // TODO: Implement printing for Conjunction constraint
        os << "Conjunction: ";
        break;

    case ConstraintKind::GenericArguments:
        // TODO: Implement printing for GenericArguments constraint
        os << "GenericArguments: ";
        break;

    case ConstraintKind::LValueObject:
        // TODO: Implement printing for LValueObject constraint
        os << "LValueObject: ";
        break;

    case ConstraintKind::ExpressibleByIntLiteral:
        // TODO: Implement printing for ExpressibleByIntLiteral constraint
        os << "ExpressibleByIntLiteral: ";
        break;

    case ConstraintKind::ExpressibleByStringLiteral:
        // TODO: Implement printing for ExpressibleByStringLiteral constraint
        os << "ExpressibleByStringLiteral: ";
        break;

    case ConstraintKind::ExpressibleByFloatLiteral:
        // TODO: Implement printing for ExpressibleByFloatLiteral constraint
        os << "ExpressibleByFloatLiteral: ";
        break;

    case ConstraintKind::ExpressibleByBoolLiteral:
        // TODO: Implement printing for ExpressibleByBoolLiteral constraint
        os << "ExpressibleByBoolLiteral: ";
        break;
    }

    os << "\n";
}

void ConstraintPrinter::print(ConstraintSystem &system, llvm::raw_ostream &os)
{
    auto const &constraints = system.getConstraints();

    os << "ConstraintSystem with " << constraints.size() << " constraint(s):\n";

    for (size_t i = 0; i < constraints.size(); ++i) {
        os << "[" << i << "] ";
        if (constraints[i]) {
            constraints[i]->print(os, 0);
        } else {
            os << "<null constraint>\n";
        }
    }

    if (constraints.empty()) {
        os << "  (no constraints)\n";
    }
}

void ConstraintPrinter::printConstraintRecursive(
    Constraint const *constraint, llvm::raw_ostream &os, unsigned indent
)
{
    if (!constraint) {
        printIndent(os, indent);
        os << "<null constraint>\n";
        return;
    }

    constraint->print(os, indent);

    // For disjunction and conjunction constraints, also print nested
    // constraints
    if (constraint->getKind() == ConstraintKind::Disjunction
        || constraint->getKind() == ConstraintKind::Conjunction) {
        auto nested = constraint->getNestedConstraints();
        for (size_t i = 0; i < nested.size(); ++i) {
            printIndent(os, indent + 1);
            os << "- [" << i << "] ";
            printConstraintRecursive(nested[i], os, indent + 1);
        }
    }
}

void ConstraintPrinter::printConstraintKind(
    ConstraintKind kind, llvm::raw_ostream &os
)
{
    // TODO: Implement constraint kind name printing
}

void ConstraintPrinter::printIndent(llvm::raw_ostream &os, unsigned indent)
{
    for (unsigned i = 0; i < indent; ++i) {
        os << "  ";
    }
}

} // namespace glu::sema
