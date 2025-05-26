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

    Type type;

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
        : ConstantInst(InstKind::IntegerLiteralInstKind), type(type)
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

    Type getType() const { return type; }
    void setType(Type newType) { this->type = newType; }

    llvm::APInt const &getValue() const
    {
        return *getTrailingObjects<llvm::APInt>();
    }

    // Direct modification of the value is not allowed after creation.
    // If necessary, create a new instance.

    Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return getType();
        case 1: return getValue();
        default: llvm_unreachable("Invalid operand index");
        }
    }

    Type getResultType([[maybe_unused]] size_t index) const override
    {
        return type;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntegerLiteralInstKind;
    }

    size_t getOperandCount() const override
    {
        return 2; // Type and APInt
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INTEGER_LITERAL_INST_HPP
