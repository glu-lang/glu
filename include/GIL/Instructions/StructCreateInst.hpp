#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP

#include "AggregateInst.hpp"
#include "Types.hpp"

#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {

/// @class StructCreateInst
/// @brief Represents an instruction to create a structure literal.
///
/// This class is derived from AggregateInst and represents an instruction
/// to create a structure literal in the GLU GIL (Glu Intermediate
/// Language).
class StructCreateInst final
    : public AggregateInst,
      private llvm::TrailingObjects<StructCreateInst, Value> {

    GLU_GIL_GEN_OPERAND(StructType, Type, _structType)
    GLU_GIL_GEN_OPERAND_LIST_TRAILING_OBJECTS(
        StructCreateInst, _fieldCount, Value, Fields
    )

    /// @brief Gets the number of fields in the structure type.
    ///
    /// @return The number of fields in the structure type.
    /// @note This method is used to determine the number of trailing objects
    ///       (values) that will be stored in the instruction.
    unsigned getFieldCount() const
    {
        return llvm::cast<types::StructTy>(_structType.getType())
            ->getFieldCount();
    }

    /// @brief Constructs a StructCreateInst object.
    ///
    /// @param structType The type of the structure being created.
    /// @param members An array of values for each member of the structure.
    StructCreateInst(Type structType, llvm::ArrayRef<Value> members)
        : AggregateInst(InstKind::StructCreateInstKind), _structType(structType)
    {
        assert(llvm::isa<types::StructTy>(structType.getType()));
        assert(
            getFieldCount() == members.size() && "Invalid number of members"
        );
        initFields(members);
    }

public:
    /// @brief Constructs a StructCreateInst object.
    ///
    /// @param structType The type of the structure being created.
    /// @param members An array of values for each member of the structure.
    static StructCreateInst *create(
        llvm::BumpPtrAllocator &alloc, Type structType,
        llvm::ArrayRef<Value> members
    )
    {
        auto totalSize = totalSizeToAlloc<Value>(members.size());
        void *mem = alloc.Allocate(totalSize, alignof(StructCreateInst));

        return new (mem) StructCreateInst(structType, members);
    }

    /// @brief Gets the structure type.
    ///
    /// @return The structure type.
    Type getStruct() const { return _structType; }

    /// @brief Gets the values of the structure members.
    ///
    /// @return An array containing the values of all structure members.
    llvm::ArrayRef<Value> getMembers() const { return getFields(); }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type (must be 0).
    /// @return The type of the created structure.
    Type getResultType() const { return _structType; }

    /// @brief Checks if the given instruction is of type StructCreateInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type StructCreateInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructCreateInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
