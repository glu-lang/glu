#include "AST/TypePrinter.hpp"
#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"

#include <cstddef>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>

namespace glu::sema {

///
/// @class ConstraintPrinter
/// @brief Utility class for printing all constraints in a ConstraintSystem.
///
class ConstraintPrinter {
    size_t _indent = 0; ///< The current indentation level.
    glu::ast::TypePrinter _typePrinter; ///< Type printer for formatting types.

public:
    ConstraintPrinter() : _typePrinter(true) { }

    /// @brief Print all constraints in a ConstraintSystem.
    /// @param system The constraint system to print.
    /// @param os The output stream to print to (defaults to stdout).
    void print(ConstraintSystem &system, llvm::raw_ostream &os = llvm::outs());

    /// @brief Print the constraint kind as a string.
    /// @param kind The constraint kind to print.
    /// @param os The output stream to print to.
    void printConstraintKind(ConstraintKind kind, llvm::raw_ostream &os);

    /// @brief Format constraint details as a string.
    /// @param constraint The constraint to format.
    /// @return A formatted string with constraint details.
    std::string formatConstraintDetails(Constraint const *constraint);

    /// @brief Format a type as a string.
    /// @param type The type to format.
    /// @return A formatted string representation of the type.
    std::string formatType(glu::types::Ty type);

    /// @brief Format a function declaration as a string.
    /// @param decl The function declaration to format.
    /// @return A formatted string representation of the function.
    std::string formatFunctionDecl(glu::ast::FunctionDecl const *decl);

    /// @brief Print statement context before constraints.
    /// @param stmt The statement to print context for.
    /// @param os The output stream to print to.
    void printStatementContext(
        glu::sema::ConstraintSystem &system, llvm::raw_ostream &os
    );

    /// @brief Helper function to print a single constraint without recursion.
    /// @param constraint The constraint to print.
    /// @param os The output stream to print to.
    /// @param indent The indentation level for formatting.
    void printSingleConstraint(
        Constraint const *constraint, llvm::raw_ostream &os, unsigned indent = 0
    );

private:
    /// @brief Recursively print a constraint and its nested constraints.
    /// @param constraint The constraint to print.
    /// @param os The output stream to print to.
    /// @param indent The indentation level for formatting.
    void printConstraintRecursive(
        Constraint const *constraint, llvm::raw_ostream &os, unsigned indent
    );
};

void Constraint::print() const
{
    // Use a single ConstraintPrinter instance
    ConstraintPrinter printer;
    printer.printSingleConstraint(this, llvm::outs());
}

void ConstraintPrinter::print(ConstraintSystem &system, llvm::raw_ostream &os)
{

    printStatementContext(system, os);

    auto const &constraints = system.getConstraints();

    os << "======== ConstraintSystem with " << constraints.size()
       << " constraint(s) ========\n";

    for (size_t i = 0; i < constraints.size(); ++i) {
        os << "[" << i << "] ";
        printConstraintRecursive(constraints[i], os, 0);
    }

    if (constraints.empty()) {
        os << "  (no constraints)\n";
    } else {
        os << "============================================="
              "==========\n";
    }
}

void ConstraintPrinter::printSingleConstraint(
    Constraint const *constraint, llvm::raw_ostream &os, unsigned indent
)
{
    if (!constraint) {
        os.indent(indent);
        os << "<null constraint>\n";
        return;
    }

    os.indent(indent);

    // Print constraint kind name
    printConstraintKind(constraint->getKind(), os);
    os << ": ";

    // Print constraint details using the structured format
    std::string details = formatConstraintDetails(constraint);
    os << details;

    os << "\n";
}

void ConstraintPrinter::printConstraintRecursive(
    Constraint const *constraint, llvm::raw_ostream &os, unsigned indent
)
{
    printSingleConstraint(constraint, os, indent);

    if (!constraint) {
        return;
    }

    // For disjunction and conjunction constraints, also print nested
    // constraints
    if (constraint->getKind() == ConstraintKind::Disjunction
        || constraint->getKind() == ConstraintKind::Conjunction) {
        auto nested = constraint->getNestedConstraints();
        for (size_t i = 0; i < nested.size(); ++i) {
            os.indent(indent + 1);
            os << "- [" << i << "] ";
            printConstraintRecursive(nested[i], os, indent + 1);
        }
    }
}

std::string
ConstraintPrinter::formatConstraintDetails(Constraint const *constraint)
{
    if (!constraint) {
        return "<null>";
    }

    std::string result;

    switch (constraint->getKind()) {
    case ConstraintKind::Bind:
    case ConstraintKind::Equal:
    case ConstraintKind::Conversion:
    case ConstraintKind::ArgumentConversion:
    case ConstraintKind::OperatorArgumentConversion:
    case ConstraintKind::CheckedCast:
    case ConstraintKind::Defaultable:
    case ConstraintKind::LValueObject:
        result = formatType(constraint->getFirstType()) + " => "
            + formatType(constraint->getSecondType());
        break;

    case ConstraintKind::BindToPointerType:
        result = formatType(constraint->getFirstType()) + " (element) <-> *"
            + formatType(constraint->getSecondType());
        break;

    case ConstraintKind::BindOverload:
        result = formatType(constraint->getOverload()) + " to overload: "
            + formatFunctionDecl(constraint->getOverloadChoice());
        break;

    case ConstraintKind::ValueMember:
    case ConstraintKind::UnresolvedValueMember:
        result = formatType(constraint->getFirstType())
            + " has member: " + formatType(constraint->getSecondType());
        if (auto member = constraint->getMember()) {
            result += " (member: " + member->getMemberName().str() + ")";
        }
        break;

    case ConstraintKind::GenericArguments:
        result = formatType(constraint->getFirstType())
            + " with generic args: " + formatType(constraint->getSecondType());
        break;

    case ConstraintKind::Disjunction:
        result = "One of "
            + std::to_string(constraint->getNestedConstraints().size())
            + " alternatives";
        break;

    case ConstraintKind::Conjunction:
        result = "All of "
            + std::to_string(constraint->getNestedConstraints().size())
            + " conditions";
        break;

    case ConstraintKind::ExpressibleByIntLiteral:
    case ConstraintKind::ExpressibleByStringLiteral:
    case ConstraintKind::ExpressibleByFloatLiteral:
    case ConstraintKind::ExpressibleByBoolLiteral:
    case ConstraintKind::StructInitialiser:
        result = formatType(constraint->getSingleType());
        break;

    case ConstraintKind::NumberOfConstraints:
        result = "<invalid-constraint-kind>";
        break;
    }

    // Add restriction info if available
    if (constraint->hasRestriction()) {
        result += " [restriction: ";
        switch (constraint->getRestriction()) {
        case ConversionRestrictionKind::DeepEquality:
            result += "DeepEquality";
            break;
        case ConversionRestrictionKind::ArrayToPointer:
            result += "ArrayToPointer";
            break;
        case ConversionRestrictionKind::StringToPointer:
            result += "StringToPointer";
            break;
        case ConversionRestrictionKind::PointerToPointer:
            result += "PointerToPointer";
            break;
        }
        result += "]";
    }

    // Add flags
    std::vector<std::string> flags;
    if (constraint->isActive())
        flags.push_back("active");
    if (constraint->isDisabled())
        flags.push_back("disabled");
    if (constraint->isFavored())
        flags.push_back("favored");
    if (constraint->isDiscarded())
        flags.push_back("discarded");
    if (constraint->shouldRememberChoice())
        flags.push_back("remember-choice");

    if (!flags.empty()) {
        result += " [";
        for (size_t i = 0; i < flags.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += flags[i];
        }
        result += "]";
    }

    // Add source location info
    if (constraint->getLocator()) {
        result += " @";
        llvm::raw_string_ostream locStream(result);
        locStream << "<AST:" << constraint->getLocator() << ">";
        locStream.flush();
    }

    return result;
}

std::string ConstraintPrinter::formatType(glu::types::Ty type)
{
    if (!type) {
        return "<null>";
    }

    return _typePrinter.visit(type);
}

std::string
ConstraintPrinter::formatFunctionDecl(glu::ast::FunctionDecl const *decl)
{
    if (!decl) {
        return "<null-function>";
    }

    std::string result = decl->getName().str();

    // Add function type information using TypePrinter
    result += ": " + formatType(decl->getType());

    return result;
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
    case ConstraintKind::StructInitialiser: os << "StructInitialiser"; break;
    case ConstraintKind::NumberOfConstraints:
        os << "NumberOfConstraints";
        break;
    }
}

void ConstraintPrinter::printStatementContext(
    glu::sema::ConstraintSystem &system, llvm::raw_ostream &os
)
{
    // Print statement context if available from the constraint system root
    if (auto *root = system.getRoot()) {

        llvm::WithColor(os, llvm::raw_ostream::MAGENTA)
            << "Constraint solving at" << " ";
        llvm::WithColor(os, llvm::raw_ostream::YELLOW)
            << "[" << system.getRoot()->getModule()->getFilePath() << "]\n";
        if (auto *stmt = llvm::dyn_cast<glu::ast::StmtBase>(root)) {
            llvm::WithColor(os, llvm::raw_ostream::MAGENTA)
                << "For statement:\n";
            os.indent(2);
            stmt->print(os);
        }
    }
}

// Public interface function
void printConstraints(ConstraintSystem &system, llvm::raw_ostream &os)
{
    ConstraintPrinter printer;
    printer.print(system, os);
}

} // namespace glu::sema
