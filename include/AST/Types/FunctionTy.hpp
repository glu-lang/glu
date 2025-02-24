#ifndef GLU_AST_TYPES_FUNCTIONTY_HPP
#define GLU_AST_TYPES_FUNCTIONTY_HPP

#include "TypeBase.hpp"

#include <vector>

namespace glu::types {

/// @brief FunctionTy is a class that represents a function type in the AST.
class FunctionTy : public TypeBase {
    std::vector<TypeBase *> const _parameters;
    TypeBase * const _returnType;

public:
    /// @brief Constructor for the FunctionTy class.
    /// @param parameters A vector of TypeBase pointers representing the
    /// parameters of the function.
    /// @param returnType A TypeBase pointer representing the return type of the
    /// function.
    FunctionTy(std::vector<TypeBase *> parameters, TypeBase *returnType)
        : TypeBase(TypeKind::FunctionTyKind)
        , _parameters(std::move(parameters))
        , _returnType(returnType)
    {
    }

    /// @brief Static function to check if a TypeBase is a FunctionTy.
    /// @param type The TypeBase to check.
    /// @return Returns true if the TypeBase is a FunctionTy, false otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::FunctionTyKind;
    }

    /// @brief Getter for a specific parameter of the function.
    /// @param index The index of the parameter to get.
    /// @return Returns a TypeBase pointer representing the parameter
    TypeBase *getParameter(std::size_t index) const
    {
        assert(index < _parameters.size() && "Index out of bounds");
        return _parameters[index];
    }

    /// @brief Getter for the number of parameters of the function.
    /// @return Returns the number of parameters
    std::size_t getParameterCount() const { return _parameters.size(); }

    /// @brief Getter for the return type of the function.
    /// @return Returns a TypeBase pointer representing the return type
    TypeBase *getReturnType() const { return _returnType; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_FUNCTIONTY_HPP
