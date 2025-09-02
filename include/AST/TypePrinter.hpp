#ifndef GLU_GIL_TYPEPRINTER_HPP
#define GLU_GIL_TYPEPRINTER_HPP

#include "AST/Types.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace glu::ast {

/// @brief TypePrinter is a visitor that converts AST types to friendly string
/// representations.
///
/// This class uses the TypeVisitor pattern to generate human-readable type
/// names for printing in GIL output. It provides friendly names for all
/// supported types.
class TypePrinter : public glu::types::TypeVisitor<TypePrinter, std::string> {
private:
    bool _enableTypeVariableNames; ///< Whether to use T1, T2, etc. for type variables
    mutable llvm::DenseMap<glu::types::TypeVariableTy *, unsigned> _typeVarIds; ///< Type variable ID mapping
    mutable unsigned _nextTypeVarId; ///< Next ID to assign to a type variable

public:
    TypePrinter(bool enableTypeVariableNames = false)
        : _enableTypeVariableNames(enableTypeVariableNames), _typeVarIds(), _nextTypeVarId(1)
    {
    }

    /// @brief Enable or disable readable type variable names (T1, T2, etc.)
    /// @param enabled Whether to enable readable type variable names
    void setTypeVariableNamesEnabled(bool enabled) { _enableTypeVariableNames = enabled; }

    /// @brief Check if readable type variable names are enabled
    /// @return True if readable type variable names are enabled
    bool isTypeVariableNamesEnabled() const { return _enableTypeVariableNames; }

    /// @brief Default fallback for unsupported types
    std::string visitTypeBase([[maybe_unused]] glu::types::TypeBase *type)
    {
        return "<unknown>";
    }

    // Built-in primitive types
    std::string visitVoidTy([[maybe_unused]] glu::types::VoidTy *type)
    {
        return "Void";
    }

    std::string visitBoolTy([[maybe_unused]] glu::types::BoolTy *type)
    {
        return "Bool";
    }

    std::string visitCharTy([[maybe_unused]] glu::types::CharTy *type)
    {
        return "Char";
    }

    std::string visitIntTy(glu::types::IntTy *type)
    {
        std::string result;
        if (type->isSigned()) {
            result = "i";
        } else {
            result = "u";
        }
        result += std::to_string(type->getBitWidth());
        return result;
    }

    std::string visitFloatTy(glu::types::FloatTy *type)
    {
        switch (type->getBitWidth()) {
        case glu::types::FloatTy::HALF: return "f16";
        case glu::types::FloatTy::FLOAT: return "f32";
        case glu::types::FloatTy::DOUBLE: return "f64";
        case glu::types::FloatTy::INTEL_LONG_DOUBLE: return "f80";
        default: return "f" + std::to_string(type->getBitWidth());
        }
    }

    // Composite types
    std::string visitPointerTy(glu::types::PointerTy *type)
    {
        return "*" + visit(type->getPointee());
    }

    std::string visitFunctionTy(glu::types::FunctionTy *type)
    {
        std::string result = "(";

        auto params = type->getParameters();
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                result += ", ";
            }
            result += visit(params[i]);
        }

        result += ") -> ";
        result += visit(type->getReturnType());
        return result;
    }

    std::string visitStaticArrayTy(glu::types::StaticArrayTy *type)
    {
        return "[" + std::to_string(type->getSize()) + " x "
            + visit(type->getDataType()) + "]";
    }

    std::string visitDynamicArrayTy(glu::types::DynamicArrayTy *type)
    {
        return "[" + visit(type->getDataType()) + "]";
    }

    std::string visitStructTy(glu::types::StructTy *type)
    {
        if (!type->getName().empty()) {
            return type->getName().str();
        }

        std::string result = "{ ";
        auto fields = type->getFields();
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) {
                result += ", ";
            }
            result += fields[i].name.str() + ": " + visit(fields[i].type);
        }
        result += " }";
        return result;
    }

    std::string visitEnumTy(glu::types::EnumTy *type)
    {
        return type->getName().str();
    }

    std::string visitTypeAliasTy(glu::types::TypeAliasTy *type)
    {
        return type->getName().str();
    }

    std::string
    visitTypeVariableTy([[maybe_unused]] glu::types::TypeVariableTy *type)
    {
        if (_enableTypeVariableNames && type) {
            // Get or assign an ID for this type variable
            auto it = _typeVarIds.find(type);
            if (it == _typeVarIds.end()) {
                _typeVarIds[type] = _nextTypeVarId++;
                it = _typeVarIds.find(type);
            }
            return "T" + std::to_string(it->second);
        }
        return "?";
    }

    std::string visitUnresolvedNameTy(glu::types::UnresolvedNameTy *type)
    {
        return type->getName().str();
    }
};

} // namespace glu::ast

#endif // GLU_GIL_TYPEPRINTER_HPP
