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

class EqualVisitor : public TypeVisitor<EqualVisitor, bool, TypeBase *> {
public:
    bool visitTypeBase(TypeBase *type, TypeBase *other)
    {
        return type->getKind() == other->getKind();
    }

    bool visitBoolTy(TypeBase *type, TypeBase *other)
    {
        return visitTypeBase(type, other);
    }

    bool visitCharTy(TypeBase *type, TypeBase *other)
    {
        return visitTypeBase(type, other);
    }

    bool visitFloatTy(FloatTy *type, TypeBase *other)
    {
        if (auto otherFloat = llvm::dyn_cast<FloatTy>(other)) {
            return type->getBitWidth() == otherFloat->getBitWidth();
        }

        return false;
    }

    bool visitFunctionTy(FunctionTy *type, TypeBase *other)
    {
        if (auto otherFunction = llvm::dyn_cast<FunctionTy>(other)) {
            if (type->getReturnType() != otherFunction->getReturnType()
                || type->getParameterCount()
                    != otherFunction->getParameterCount()) {
                return false;
            }

            for (std::size_t i = 0, n = type->getParameterCount(); i < n; ++i) {
                if (type->getParameter(i) != otherFunction->getParameter(i))
                    return false;
            }

            return true;
        }
        return false;
    }

    bool visitIntTy(IntTy *type, TypeBase *other)
    {
        if (auto otherInt = llvm::dyn_cast<IntTy>(other)) {
            return type->getSignedness() == otherInt->getSignedness()
                && type->getBitWidth() == otherInt->getBitWidth();
        }

        return false;
    }
};

unsigned glu::types::TypeBase::hash(TypeBase const *T) const
{
    return HashVisitor().visit(const_cast<TypeBase *>(T));
}

bool glu::types::TypeBase::operator==(TypeBase const &other) const
{
    return EqualVisitor().visit(
        const_cast<TypeBase *>(this), const_cast<TypeBase *>(&other)
    );
}
