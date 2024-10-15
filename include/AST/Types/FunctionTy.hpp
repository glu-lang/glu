#ifndef GLU_AST_TYPES_FUNCTIONTY_HPP
#define GLU_AST_TYPES_FUNCTIONTY_HPP

#include "TypeBase.hpp"
#include <vector>

namespace glu::types {

/// @brief FunctionTy is a class that represents a function type in the AST.
class FunctionTy : public TypeBase {
private:
    std::vector<TypeBase *> const _parameters;
    TypeBase *_returnType;

public:
    /// @brief Constructor for the FunctionTy class.
    /// @param parameters A vector of TypeBase pointers representing the
    /// parameters of the function.
    /// @param returnType A TypeBase pointer representing the return type of the
    /// function.
    FunctionTy(std::vector<TypeBase *> const &&parameters, TypeBase *returnType)
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

    /// @brief Getter for the parameters of the function.
    /// @return Returns a vector of TypeBase pointers representing the
    /// parameters
    std::vector<TypeBase *> const &getParameters() const { return _parameters; }

    /// @brief Getter for the return type of the function.
    /// @return Returns a TypeBase pointer representing the return type
    TypeBase *getReturnType() const { return _returnType; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_FUNCTIONTY_HPP
