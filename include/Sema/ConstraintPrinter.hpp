#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"

#include <cstddef>
#include <llvm/Support/raw_ostream.h>

namespace glu::sema {

///
/// @class ConstraintPrinter
/// @brief Utility class for printing all constraints in a ConstraintSystem.
///
class ConstraintPrinter {
    size_t _indent = 0; ///< The current indentation level.

public:
    /// @brief Print all constraints in a ConstraintSystem.
    /// @param system The constraint system to print.
    /// @param os The output stream to print to (defaults to stdout).
    static void
    print(ConstraintSystem &system, llvm::raw_ostream &os = llvm::outs());

    /// @brief Print indentation spaces.
    /// @param os The output stream to print to.
    /// @param indent The number of indent levels.
    static void printIndent(llvm::raw_ostream &os, unsigned indent);

private:
    /// @brief Print the constraint kind as a string.
    /// @param kind The constraint kind to print.
    /// @param os The output stream to print to.
    static void printConstraintKind(ConstraintKind kind, llvm::raw_ostream &os);

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
    ConstraintPrinter::printIndent(os, indent);

    switch (getKind()) {
    case ConstraintKind::Bind:
        os << "bind ";
        getFirstType()->print();
        os << " := ";
        getSecondType()->print();
        break;

    case ConstraintKind::Equal:
        os << "equal ";
        getFirstType()->print();
        os << " == ";
        getSecondType()->print();
        break;

    case ConstraintKind::BindToPointerType:
        os << "bind-to-pointer ";
        getFirstType()->print();
        os << " <: *";
        getSecondType()->print();
        break;

    case ConstraintKind::Conversion:
        os << "conversion ";
        getFirstType()->print();
        os << " ~> ";
        getSecondType()->print();
        if (hasRestriction()) {
            os << " [";
            switch (getRestriction()) {
            case ConversionRestrictionKind::DeepEquality:
                os << "deep-equality";
                break;
            case ConversionRestrictionKind::ArrayToPointer:
                os << "array-to-pointer";
                break;
            case ConversionRestrictionKind::StringToPointer:
                os << "string-to-pointer";
                break;
            case ConversionRestrictionKind::PointerToPointer:
                os << "pointer-to-pointer";
                break;
            }
            os << "]";
        }
        break;

    case ConstraintKind::ArgumentConversion:
        os << "argument ";
        getFirstType()->print();
        os << " ~arg> ";
        getSecondType()->print();
        if (hasRestriction()) {
            os << " [restriction]";
        }
        break;

    case ConstraintKind::OperatorArgumentConversion:
        os << "operator-arg ";
        getFirstType()->print();
        os << " ~op> ";
        getSecondType()->print();
        if (hasRestriction()) {
            os << " [restriction]";
        }
        break;

    case ConstraintKind::CheckedCast:
        os << "checked-cast ";
        getFirstType()->print();
        os << " as ";
        getSecondType()->print();
        if (hasRestriction()) {
            os << " [restricted]";
        }
        break;

    case ConstraintKind::BindOverload:
        os << "bind-overload ";
        getOverload()->print();
        os << " to choice: ";
        if (auto *choice = getOverloadChoice()) {
            os << choice->getName();
        } else {
            os << "<unknown>";
        }
        break;

    case ConstraintKind::ValueMember:
        os << "value-member ";
        getFirstType()->print();
        os << ".";
        if (auto *member = getMember()) {
            os << member->getMemberName();
        } else {
            os << "<unknown-member>";
        }
        os << " : ";
        getSecondType()->print();
        break;

    case ConstraintKind::UnresolvedValueMember:
        os << "unresolved-member ";
        getFirstType()->print();
        os << ".<?> : ";
        getSecondType()->print();
        if (auto *member = getMember()) {
            os << " (candidate: " << member->getMemberName() << ")";
        }
        break;

    case ConstraintKind::Defaultable:
        os << "defaultable ";
        getFirstType()->print();
        os << " ?: ";
        getSecondType()->print();
        break;

    case ConstraintKind::Disjunction:
        os << "disjunction {" << getNestedConstraints().size() << " choices}";
        if (shouldRememberChoice()) {
            os << " [remember-choice]";
        }
        break;

    case ConstraintKind::Conjunction:
        os << "conjunction {" << getNestedConstraints().size()
           << " requirements}";
        break;

    case ConstraintKind::GenericArguments:
        os << "generic-args ";
        getFirstType()->print();
        os << " <: ";
        getSecondType()->print();
        break;

    case ConstraintKind::LValueObject:
        os << "lvalue ";
        getFirstType()->print();
        os << " -> ";
        getSecondType()->print();
        break;

    case ConstraintKind::ExpressibleByIntLiteral:
        os << "literal-conformance ";
        getSingleType()->print();
        os << " : ExpressibleByIntLiteral";
        break;

    case ConstraintKind::ExpressibleByStringLiteral:
        os << "literal-conformance ";
        getSingleType()->print();
        os << " : ExpressibleByStringLiteral";
        break;

    case ConstraintKind::ExpressibleByFloatLiteral:
        os << "literal-conformance ";
        getSingleType()->print();
        os << " : ExpressibleByFloatLiteral";
        break;

    case ConstraintKind::ExpressibleByBoolLiteral:
        os << "literal-conformance ";
        getSingleType()->print();
        os << " : ExpressibleByBoolLiteral";
        break;

    case ConstraintKind::NumberOfConstraints:
        os << "<invalid-constraint-kind>";
        break;
    }

    // Print source location info
    if (getLocator()) {
        os << " @";
        // Simple placeholder for location - you might want to enhance this
        os << "<AST:" << static_cast<void *>(getLocator()) << ">";
    }

    os << "\n";
}

void ConstraintPrinter::print(ConstraintSystem &system, llvm::raw_ostream &os)
{
    auto const &constraints = system.getConstraints();

    os << "======== ConstraintSystem with " << constraints.size()
       << " constraint(s) ========\n";

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
    switch (kind) {
    case ConstraintKind::Bind: os << "Bind"; break;
    case ConstraintKind::Equal: os << "Equal"; break;
    case ConstraintKind::BindToPointerType: os << "BindToPointerType"; break;
    case ConstraintKind::Conversion: os << "Conversion"; break;
    case ConstraintKind::ArgumentConversion: os << "ArgumentConversion"; break;
    case ConstraintKind::OperatorArgumentConversion:
        os << "OperatorArgumentConversion";
        break;
    case ConstraintKind::CheckedCast: os << "CheckedCast"; break;
    case ConstraintKind::BindOverload: os << "BindOverload"; break;
    case ConstraintKind::ValueMember: os << "ValueMember"; break;
    case ConstraintKind::UnresolvedValueMember:
        os << "UnresolvedValueMember";
        break;
    case ConstraintKind::Defaultable: os << "Defaultable"; break;
    case ConstraintKind::Disjunction: os << "Disjunction"; break;
    case ConstraintKind::Conjunction: os << "Conjunction"; break;
    case ConstraintKind::GenericArguments: os << "GenericArguments"; break;
    case ConstraintKind::LValueObject: os << "LValueObject"; break;
    case ConstraintKind::ExpressibleByIntLiteral:
        os << "ExpressibleByIntLiteral";
        break;
    case ConstraintKind::ExpressibleByStringLiteral:
        os << "ExpressibleByStringLiteral";
        break;
    case ConstraintKind::ExpressibleByFloatLiteral:
        os << "ExpressibleByFloatLiteral";
        break;
    case ConstraintKind::ExpressibleByBoolLiteral:
        os << "ExpressibleByBoolLiteral";
        break;
    case ConstraintKind::NumberOfConstraints:
        os << "NumberOfConstraints";
        break;
    }
}

void ConstraintPrinter::printIndent(llvm::raw_ostream &os, unsigned indent)
{
    for (unsigned i = 0; i < indent; ++i) {
        os << "  ";
    }
}

} // namespace glu::sema
