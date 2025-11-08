#ifndef GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP

#include "ConstantInst.hpp"
#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {

class IntegerLiteralInst final
    : public ConstantInst,
      private llvm::TrailingObjects<IntegerLiteralInst, llvm::APInt> {
    using TrailingArgs = llvm::TrailingObjects<IntegerLiteralInst, llvm::APInt>;
    friend TrailingArgs;

    GLU_GIL_GEN_OPERAND(Type, Type, _type)

    // Method required by TrailingObjects to determine the number of trailing
    // objects
    size_t numTrailingObjects(
        typename TrailingArgs::OverloadToken<llvm::APInt>
    ) const
    {
        return 1; // Always 1 APInt
    }

    // Private constructor
    IntegerLiteralInst(Type type, llvm::APInt const &value)
        : ConstantInst(InstKind::IntegerLiteralInstKind), _type(type)
    {
        // Use the copy constructor of APInt to initialize the trailing object
        new (getTrailingObjects<llvm::APInt>()) llvm::APInt(value);
    }

public:
    static IntegerLiteralInst *create(
        llvm::BumpPtrAllocator &allocator, Type type, llvm::APInt const &value
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<llvm::APInt>(1), alignof(IntegerLiteralInst)
        );
        return new (mem) IntegerLiteralInst(type, value);
    }

    llvm::APInt const &getValue() const
    {
        return *getTrailingObjects<llvm::APInt>();
    }

    // Direct modification of the value is not allowed after creation.
    // If necessary, create a new instance.

    Type getResultType() const { return _type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntegerLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
