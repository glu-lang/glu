#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP

#include "ConstantInst.hpp"
#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {

class FloatLiteralInst final
    : public ConstantInst,
      private llvm::TrailingObjects<FloatLiteralInst, llvm::APFloat> {
    using TrailingArgs = llvm::TrailingObjects<FloatLiteralInst, llvm::APFloat>;
    friend TrailingArgs;

    GLU_GIL_GEN_OPERAND(Type, Type, _type)

    // Method required by TrailingObjects to determine the number of trailing
    // objects
    size_t numTrailingObjects(
        typename TrailingArgs::OverloadToken<llvm::APFloat>
    ) const
    {
        return 1; // Always 1 APFloat
    }

    // Private constructor
    FloatLiteralInst(Type type, llvm::APFloat const &value)
        : ConstantInst(InstKind::FloatLiteralInstKind), _type(type)
    {
        // Use the copy constructor of APFloat to initialize the trailing object
        new (getTrailingObjects<llvm::APFloat>()) llvm::APFloat(value);
    }

public:
    static FloatLiteralInst *create(
        llvm::BumpPtrAllocator &allocator, Type type, llvm::APFloat const &value
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<llvm::APFloat>(1), alignof(FloatLiteralInst)
        );
        return new (mem) FloatLiteralInst(type, value);
    }

    llvm::APFloat const &getValue() const
    {
        return *getTrailingObjects<llvm::APFloat>();
    }

    // Direct modification of the value is not allowed after creation.
    // If necessary, create a new instance.

    Type getResultType() const { return _type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FloatLiteralInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_LITERAL_INST_HPP
