#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP

#include "AST/Decls.hpp"
#include "AggregateInst.hpp"
#include "Member.hpp"
#include "Type.hpp"
#include "Value.hpp"

#include "AST/Types/StructTy.hpp"
#include <llvm/Support/TrailingObjects.h>

namespace glu::gil {

/// @class StructDestructureInst
/// @brief Instruction that destructures a struct value into its individual
/// fields.
///
/// It produces N results, where N is the number of fields in the struct.
/// The only operand is the struct value.
class StructDestructureInst final
    : public AggregateInst,
      private llvm::TrailingObjects<StructDestructureInst, Type> {

    using TrailingFieldTypes
        = llvm::TrailingObjects<StructDestructureInst, Type>;
    friend TrailingFieldTypes;

    Value _structValue; ///< The value of the struct being destructured.
    unsigned _fieldCount; ///< Number of fields in the struct.

    // Method required by llvm::TrailingObjects to determine the number of
    // trailing objects.
    size_t numTrailingObjects(
        typename TrailingFieldTypes::OverloadToken<Type>
    ) const
    {
        return _fieldCount;
    }

private:
    /// @brief Private constructor for the StructDestructureInst that takes
    /// trailing objects.
    ///
    /// @param structValue The struct value to destructure.
    /// @param fieldTypes Pre-computed types for each field with real
    /// size/alignment.
    StructDestructureInst(Value structValue, llvm::ArrayRef<Type> fieldTypes)
        : AggregateInst(InstKind::StructDestructureInstKind)
        , _structValue(structValue)
        , _fieldCount(fieldTypes.size())
    {
        assert(
            llvm::isa<glu::types::StructTy>(structValue.getType().getType())
            && "StructDestructureInst requires a struct-typed value"
        );

        auto structTy
            = llvm::cast<glu::types::StructTy>(structValue.getType().getType());
        assert(
            _fieldCount == structTy->getFieldCount()
            && "Number of field types must match struct field count"
        );

        // Use uninitialized_copy for raw memory
        std::uninitialized_copy(
            fieldTypes.begin(), fieldTypes.end(), getFieldTypesPtr()
        );
    }

public:
    /// @brief Static factory method to create a StructDestructureInst.
    ///
    /// @param arena The memory arena to allocate from.
    /// @param structValue The struct value to destructure.
    /// @param fieldTypes Pre-computed types for each field with real
    /// size/alignment.
    static StructDestructureInst *create(
        llvm::BumpPtrAllocator &arena, Value structValue,
        llvm::ArrayRef<Type> fieldTypes
    )
    {
        auto totalSize = totalSizeToAlloc<Type>(fieldTypes.size());
        void *mem = arena.Allocate(totalSize, alignof(StructDestructureInst));

        return new (mem) StructDestructureInst(structValue, fieldTypes);
    }

    // Helper methods to access the trailing objects
    Type *getFieldTypesPtr() { return getTrailingObjects<Type>(); }
    Type const *getFieldTypesPtr() const { return getTrailingObjects<Type>(); }

    llvm::ArrayRef<Type> getFieldTypes() const
    {
        return llvm::ArrayRef<Type>(getFieldTypesPtr(), _fieldCount);
    }

    /// @brief Gets the struct value operand.
    Value getStructValue() const { return _structValue; }

    /// @brief Gets the number of operands (always 1).
    size_t getOperandCount() const override { return 1; }

    /// @brief Gets the operand at the given index.
    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return Operand(_structValue);
        llvm_unreachable("Invalid operand index");
    }

    /// @brief Gets the number of results (equal to number of struct fields).
    size_t getResultCount() const override
    {
        auto structTy
            = llvm::cast<glu::types::StructTy>(_structValue.getType().getType()
            );
        return structTy->getFieldCount();
    }

    /// @brief Gets the result type at a given index (type of each field).
    Type getResultType(size_t index) const override
    {
        auto structTy
            = llvm::cast<glu::types::StructTy>(_structValue.getType().getType()
            );
        assert(
            index < structTy->getFieldCount()
            && "Field index out of bounds for "
               "StructDestructureInst::getResultType"
        );

        // Return the pre-computed field type with real size, alignment, and
        // constness
        return getFieldTypesPtr()[index];
    }

    /// @brief Gets the list of members (fields).
    std::vector<Member> getMembers() const
    {
        std::vector<Member> membersVec;
        auto gilStructType = _structValue.getType();
        auto astStructTypeNode = gilStructType.getType();

        assert(astStructTypeNode && "Underlying AST TypeBase node is null");
        auto astStructType
            = llvm::cast<glu::types::StructTy>(astStructTypeNode);

        size_t fieldCount = astStructType->getFieldCount();
        membersVec.reserve(fieldCount);

        for (size_t i = 0; i < fieldCount; ++i) {
            glu::ast::FieldDecl *astField = astStructType->getField(i);
            // getResultType(i) will provide the gil::Type for the field,
            // with the actual computed size/alignment from the constructor
            Type fieldGilType = getResultType(i);
            membersVec.emplace_back(
                astField->getName().str(), fieldGilType, gilStructType
            );
        }
        return membersVec;
    }

    /// @brief Checks if an instruction is a StructDestructureInst.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructDestructureInstKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP
