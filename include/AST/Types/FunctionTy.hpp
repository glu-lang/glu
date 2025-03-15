#ifndef GLU_AST_TYPES_FUNCTIONTY_HPP
#define GLU_AST_TYPES_FUNCTIONTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

/// @brief FunctionTy is a class that represents a function type in the AST.
class FunctionTy final : public TypeBase,
                         private llvm::TrailingObjects<FunctionTy, TypeBase *> {
    using TrailingParams = llvm::TrailingObjects<FunctionTy, TypeBase *>;

public:
    friend TrailingParams;
    friend class InternedMemoryArena<TypeBase>;

private:
    TypeBase * const _returnType;
    unsigned _numParams;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename TrailingParams::OverloadToken<TypeBase *>)
            const
    {
        return _numParams;
    }

    FunctionTy(unsigned numParams, TypeBase *returnType)
        : TypeBase(TypeKind::FunctionTyKind)
        , _returnType(returnType)
        , _numParams(numParams)
    {
    }

    FunctionTy(llvm::ArrayRef<TypeBase *> params, TypeBase *returnType)
        : FunctionTy(params.size(), returnType)
    {
        std::uninitialized_copy(
            params.begin(), params.end(),
            this->template getTrailingObjects<TypeBase *>()
        );
    }

public:
    /// @brief Constructor for the FunctionTy class.
    /// @param params A vector of TypeBase pointers representing the
    /// parameters of the function.
    /// @param returnType A TypeBase pointer representing the return type of
    /// the function.
    static FunctionTy *create(
        llvm::BumpPtrAllocator &allocator, llvm::ArrayRef<TypeBase *> params,
        TypeBase *returnType
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<TypeBase *>(params.size()), alignof(FunctionTy)
        );

        return new (mem) FunctionTy(params, returnType);
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

    /// @brief Getter for the return type of the function.
    /// @return Returns a TypeBase pointer representing the return type
    TypeBase *getReturnType() const { return _returnType; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_FUNCTIONTY_HPP
