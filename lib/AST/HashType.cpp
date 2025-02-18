#include "Types.hpp"

#include <llvm/ADT/Hashing.h>

using namespace glu::types;

class HashVisitor : public TypeVisitor<HashVisitor, std::size_t> {
public:
    std::size_t visitTypeBase(TypeBase *type)
    {
        return llvm::hash_value(type->getKind());
    }

    std::size_t visitBoolTy(TypeBase *type) { return visitTypeBase(type); }

    std::size_t visitCharTy(TypeBase *type) { return visitTypeBase(type); }

    std::size_t visitFloatTy(FloatTy *type)
    {
        return llvm::hash_combine(type->getKind(), type->getBitWidth());
    }

    std::size_t visitFunctionTy(FunctionTy *type)
    {
        std::size_t hash
            = llvm::hash_combine(type->getKind(), type->getReturnType());

        auto parameters = type->getParameterCount();
        for (std::size_t i = 0; i < parameters; ++i) {
            hash = llvm::hash_combine(hash, type->getParameter(i));
        }

        return hash;
    };

    std::size_t visitIntTy(IntTy *type)
    {
        return llvm::hash_combine(
            type->getKind(), type->getSignedness(), type->getBitWidth()
        );
    }
};

unsigned glu::types::TypeBase::hash(TypeBase const *T) const
{
    return HashVisitor().visit(const_cast<TypeBase *>(T));
}
