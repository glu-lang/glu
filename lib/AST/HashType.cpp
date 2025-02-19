#include "Types.hpp"

#include <llvm/ADT/Hashing.h>

using namespace glu::types;

class HashVisitor : public TypeVisitor<HashVisitor, std::size_t> {
public:
    std::size_t visitTypeBase(TypeBase *type)
    {
        return llvm::hash_value(type->getKind());
    }

    std::size_t visitDynamicArrayTy(DynamicArrayTy *type)
    {
        return llvm::hash_combine(type->getKind(), type->getDataKind());
    }

    std::size_t visitEnumTy(EnumTy *type)
    {
        std::size_t hash = llvm::hash_combine(type->getKind(), type->getName());

        for (std::size_t i = 0, n = type->getCaseCount(); i < n; ++i) {
            hash = llvm::hash_combine(
                hash, type->getCase(i).name, type->getCase(i).value
            );
        }

        return hash;
    }

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

    std::size_t visitPointerTy(PointerTy *type)
    {
        return llvm::hash_combine(type->getKind(), type->getPointee());
    }

    std::size_t visitStaticArrayTy(StaticArrayTy *type)
    {
        return llvm::hash_combine(
            type->getKind(), type->getDataKind(), type->getSize()
        );
    }

    std::size_t visitStructTy(StructTy *type)
    {
        std::size_t hash = llvm::hash_combine(type->getKind(), type->getName());

        for (std::size_t i = 0, n = type->getFieldCount(); i < n; ++i) {
            hash = llvm::hash_combine(
                hash, type->getField(i).name, type->getField(i).type
            );
        }

        return hash;
    }

    std::size_t visitTypeAliasTy(TypeAliasTy *type)
    {
        return llvm::hash_combine(
            type->getKind(), type->getWrappedType(), type->getName()
        );
    }

    std::size_t visitUnresolvedNameTy(UnresolvedNameTy *type)
    {
        return llvm::hash_combine(type->getKind(), type->getName());
    }
};

class EqualVisitor : public TypeVisitor<EqualVisitor, bool, TypeBase *> {
public:
    bool visitTypeBase(TypeBase *type, TypeBase *other)
    {
        return type->getKind() == other->getKind();
    }

    bool visitDynamicArrayTy(DynamicArrayTy *type, TypeBase *other)
    {
        if (auto otherArray = llvm::dyn_cast<DynamicArrayTy>(other)) {
            return type->getDataKind() == otherArray->getDataKind();
        }

        return false;
    }

    bool visitEnumTy(EnumTy *type, TypeBase *other)
    {
        if (auto otherEnum = llvm::dyn_cast<EnumTy>(other)) {
            return type->getDefinitionLocation()
                == otherEnum->getDefinitionLocation();
        }
        return false;
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

    bool visitPointerTy(PointerTy *type, TypeBase *other)
    {
        if (auto otherPointer = llvm::dyn_cast<PointerTy>(other)) {
            return type->getPointee() == otherPointer->getPointee();
        }

        return false;
    }

    bool visitStaticArrayTy(StaticArrayTy *type, TypeBase *other)
    {
        if (auto otherArray = llvm::dyn_cast<StaticArrayTy>(other)) {
            return type->getDataKind() == otherArray->getDataKind()
                && type->getSize() == otherArray->getSize();
        }

        return false;
    }

    bool visitStructTy(StructTy *type, TypeBase *other)
    {
        if (auto otherStruct = llvm::dyn_cast<StructTy>(other)) {
            return type->getDefinitionLocation()
                == otherStruct->getDefinitionLocation();
        }

        return false;
    }

    bool visitTypeAliasTy(TypeAliasTy *type, TypeBase *other)
    {
        if (auto otherAlias = llvm::dyn_cast<TypeAliasTy>(other)) {
            return type->getLocation() == otherAlias->getLocation();
        }

        return false;
    }

    bool visitUnresolvedNameTy(UnresolvedNameTy *type, TypeBase *other)
    {
        if (auto otherName = llvm::dyn_cast<UnresolvedNameTy>(other)) {
            return type->getName() == otherName->getName();
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
