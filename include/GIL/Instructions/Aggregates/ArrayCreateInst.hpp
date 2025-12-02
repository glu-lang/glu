#ifndef GLU_GIL_INSTRUCTIONS_ARRAY_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_ARRAY_CREATE_HPP

#include "AggregateInst.hpp"
#include "Types.hpp"

#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {

/// @class ArrayCreateInst
/// @brief Represents an instruction that creates a static array literal.
class ArrayCreateInst final
    : public AggregateInst,
      private llvm::TrailingObjects<ArrayCreateInst, Value> {

    GLU_GIL_GEN_OPERAND(ArrayType, Type, _arrayType)
    GLU_GIL_GEN_OPERAND_LIST_TRAILING_OBJECTS(
        ArrayCreateInst, _elementCount, Value, Elements
    )

    std::size_t getExpectedElementCount() const
    {
        auto *arrayTy = llvm::cast<types::StaticArrayTy>(_arrayType);
        return arrayTy->getSize();
    }

    ArrayCreateInst(Type arrayType, llvm::ArrayRef<Value> elements)
        : AggregateInst(InstKind::ArrayCreateInstKind), _arrayType(arrayType)
    {
        assert(
            llvm::isa<types::StaticArrayTy>(arrayType)
            && "array_create requires a static array type"
        );
        assert(
            getExpectedElementCount() == elements.size()
            && "element count must match array type"
        );
        initElements(elements);
    }

public:
    static ArrayCreateInst *
    create(Type arrayType, llvm::ArrayRef<Value> elements)
    {
        void *mem = ::operator new(totalSizeToAlloc<Value>(elements.size()));
        return new (mem) ArrayCreateInst(arrayType, elements);
    }

    void operator delete(void *ptr) { ::operator delete(ptr); }

    Type getResultType() const { return _arrayType; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::ArrayCreateInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ARRAY_CREATE_HPP
