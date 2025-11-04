#ifndef GLU_GILGEN_LITERALVISITOR_HPP
#define GLU_GILGEN_LITERALVISITOR_HPP

#include "Context.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <variant>

#include "AST/Exprs.hpp"

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
    gil::Value visit(ast::LiteralExpr::LiteralValue const &value)
    {
        return std::visit(
            [this](auto const &v) { return this->visit(v); }, value
        );
    }

    static llvm::APFloat createZero(types::FloatTy *ty)
    {
        if (ty->isFloat()) {
            return llvm::APFloat::IEEEsingle();
        } else if (ty->isDouble()) {
            return llvm::APFloat::IEEEdouble();
        } else if (ty->isHalf()) {
            return llvm::APFloat::IEEEhalf();
        } else if (ty->isIntelLongDouble()) {
            return llvm::APFloat::x87DoubleExtended();
        } else {
            llvm_unreachable("Unsupported float width");
        }
    }

    ///
    /// @brief Handle integer literals
    ///
    /// @param value The integer value
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(llvm::APInt const &value)
    {
        if (types::IntTy *intTy = llvm::dyn_cast<types::IntTy>(&*_type)) {
            llvm::APInt intValue = value.zextOrTrunc(intTy->getBitWidth());
            return _ctx.buildIntegerLiteral(_type, intValue)->getResult(0);
        } else if (types::FloatTy *floatTy
                   = llvm::dyn_cast<types::FloatTy>(&*_type)) {
            // Convert integer to float
            llvm::APFloat floatValue = createZero(floatTy);
            floatValue.convertFromAPInt(
                value, /*isSigned=*/true, llvm::APFloat::rmNearestTiesToEven
            );
            return _ctx.buildFloatLiteral(_type, floatValue)->getResult(0);
        }
        llvm_unreachable("Unsupported type for integer literal");
    }

    ///
    /// @brief Handle floating-point literals
    ///
    /// @param value The floating-point value
    /// @return gil::Value The resulting GIL value
    ///
    gil::Value visit(llvm::APFloat const &value)
    {
        types::FloatTy *floatTy = llvm::dyn_cast<types::FloatTy>(&*_type);
        if (!floatTy) {
            llvm_unreachable("Unsupported type for float literal");
        }

        // Convert to the correct float semantics if needed
        llvm::APFloat semantics = createZero(floatTy);
        llvm::APFloat floatValue = value;
        bool losesInfo;
        floatValue.convert(
            semantics.getSemantics(), llvm::APFloat::rmNearestTiesToEven,
            &losesInfo
        );
        return _ctx.buildFloatLiteral(_type, floatValue)->getResult(0);
    }

    gil::Value visit(std::nullptr_t)
    {
        auto intValue = llvm::APInt(64, 0);
        auto *u64 = _ctx.getASTFunction()
                        ->getModule()
                        ->getContext()
                        ->getTypesMemoryArena()
                        .create<glu::types::IntTy>(
                            glu::types::IntTy(glu::types::IntTy::Unsigned, 64)
                        );

        auto *zero
            = _ctx.buildIntegerLiteral(_ctx.translateType(u64), intValue);
        return _ctx.buildCastIntToPtr(_type, zero->getResult(0))->getResult(0);
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
