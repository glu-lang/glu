#ifndef GLU_GILGEN_LITERALVISITOR_HPP
#define GLU_GILGEN_LITERALVISITOR_HPP

#include "Context.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <variant>

namespace glu::gilgen {

///
/// @class LiteralVisitor
/// @brief Visitor for converting literal values into GIL instructions
///
/// This visitor is used to generate GIL instructions for different types of
/// literals. It follows the same visitor pattern as other visitors in the
/// project.
///
class LiteralVisitor {
    Context &_ctx;
    gil::Type _type;

public:
    ///
    /// @brief Construct a new Literal Visitor
    ///
    /// @param ctx Context in which to build instructions
    /// @param type GIL type of the literal
    ///
    LiteralVisitor(Context &ctx, gil::Type type) : _ctx(ctx), _type(type) { }

    ///
    /// @brief Visit a literal value to generate appropriate GIL instructions
    ///
    /// @param value The literal value as a variant
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(
        std::variant<llvm::APInt, llvm::APFloat, llvm::StringRef, bool> const
            &value
    )
    {
        return std::visit(
            [this](auto const &v) { return this->visit(v); }, value
        );
    }

    ///
    /// @brief Handle integer literals
    ///
    /// @param value The integer value
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(llvm::APInt const &value)
    {
        return _ctx.buildIntegerLiteral(_type, value)->getResult(0);
    }

    ///
    /// @brief Handle floating-point literals
    ///
    /// @param value The floating-point value
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(llvm::APFloat const &value)
    {
        return _ctx.buildFloatLiteral(_type, value)->getResult(0);
    }

    ///
    /// @brief Handle boolean literals
    ///
    /// @param value The boolean value
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(bool value)
    {
        return _ctx.buildBoolLiteral(_type, value)->getResult(0);
    }

    ///
    /// @brief Handle string literals
    ///
    /// @param value The string value
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(llvm::StringRef value)
    {
        return _ctx.buildStringLiteral(_type, value)->getResult(0);
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_LITERALVISITOR_HPP
