#ifndef GLU_AST_TYPES_FUNCTIONTY_HPP
#define GLU_AST_TYPES_FUNCTIONTY_HPP

#include "TypeBase.hpp"

#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

#include <optional>

namespace glu::types {

/// @brief FunctionTy is a class that represents a function type in the AST.
class FunctionTy final : public TypeBase,
                         private llvm::TrailingObjects<FunctionTy, TypeBase *> {
    using TrailingParams = llvm::TrailingObjects<FunctionTy, TypeBase *>;

public:
    friend TrailingParams;

private:
    TypeBase * const _returnType;
    unsigned _numParams;
    unsigned _requiredParamCount;
    bool _isCVariadic = false;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(
        typename TrailingParams::OverloadToken<TypeBase *>
    ) const
    {
        return _numParams;
    }

    FunctionTy(
        llvm::ArrayRef<TypeBase *> params, TypeBase *returnType,
        bool isCVariadic, unsigned requiredParamCount
    )
        : TypeBase(TypeKind::FunctionTyKind)
        , _returnType(returnType)
        , _numParams(params.size())
        , _requiredParamCount(requiredParamCount)
        , _isCVariadic(isCVariadic)
    {
        std::uninitialized_copy(
            params.begin(), params.end(), getTrailingObjects<TypeBase *>()
        );
    }

    /// @brief Constructor with default required param count (all params
    /// required).
    FunctionTy(
        llvm::ArrayRef<TypeBase *> params, TypeBase *returnType,
        bool isCVariadic = false
    )
        : FunctionTy(params, returnType, isCVariadic, params.size())
    {
    }

public:
    /// @brief Constructor for the FunctionTy class.
    /// @param params A vector of TypeBase pointers representing the
    /// parameters of the function.
    /// @param returnType A TypeBase pointer representing the return type of
    /// the function.
    /// @param isCVariadic Whether the function is C variadic.
    /// @param requiredParamCount The number of required parameters. If not
    /// specified, defaults to the total number of parameters.
    static FunctionTy *create(
        llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<TypeBase *> params,
        TypeBase *returnType, bool isCVariadic = false,
        std::optional<unsigned> requiredParamCount = std::nullopt
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<TypeBase *>(params.size()), alignof(FunctionTy)
        );

        unsigned required = requiredParamCount.value_or(params.size());
        return new (mem) FunctionTy(params, returnType, isCVariadic, required);
    }

    /// @brief Static function to check if a TypeBase is a FunctionTy.
    /// @param type The TypeBase to check.
    /// @return Returns true if the TypeBase is a FunctionTy, false
    /// otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::FunctionTyKind;
    }

    /// @brief Getter for a specific parameter of the function.
    /// @param index The index of the parameter to get.
    /// @return Returns a TypeBase pointer representing the parameter
    TypeBase *getParameter(std::size_t index) const
    {
        assert(index < _numParams && "Index out of bounds");

        return getTrailingObjects<TypeBase *>()[index];
    }

    /// @brief Getter for all parameters of the function.
    /// @return Returns an ArrayRef of TypeBase pointers representing the
    /// function's parameters.
    llvm::ArrayRef<TypeBase *> getParameters() const
    {
        return { getTrailingObjects<TypeBase *>(), _numParams };
    }

    /// @brief Getter for the number of parameters of the function.
    /// @return Returns the number of parameters
    std::size_t getParameterCount() const { return _numParams; }

    /// @brief Getter for the number of required parameters.
    /// @return Returns the number of required parameters (those without
    /// default values).
    std::size_t getRequiredParameterCount() const
    {
        return _requiredParamCount;
    }

    /// @brief Getter for the return type of the function.
    /// @return Returns a TypeBase pointer representing the return type
    TypeBase *getReturnType() const { return _returnType; }

    /// @brief Getter for the CVariadic property of the function type.
    /// @return Returns true if the function is CVariadic, false otherwise.
    bool isCVariadic() const { return _isCVariadic; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_FUNCTIONTY_HPP
