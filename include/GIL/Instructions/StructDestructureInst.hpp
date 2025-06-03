#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP

#include "AggregateInst.hpp"
#include "Member.hpp"
#include "Type.hpp"
#include "Value.hpp"

#include "AST/Types/StructTy.hpp"
#include <vector>

namespace glu::gil {

/// @class StructDestructureInst
/// @brief Instruction that destructures a struct value into its individual
/// fields.
///
/// It produces N results, where N is the number of fields in the struct.
/// The only operand is the struct value.
class StructDestructureInst : public AggregateInst {
    Value _structValue; ///< The value of the struct being destructured.

public:
    /// @brief Constructs a StructDestructureInst.
    ///
    /// @param structValue The struct value to destructure.
    StructDestructureInst(Value structValue)
        : AggregateInst(InstKind::StructDestructureInstKind)
        , _structValue(structValue)
    {
        assert(
            llvm::isa<glu::types::StructTy>(structValue.getType().getType())
            && "StructDestructureInst requires a struct-typed value"
        );
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
        auto gilStructType = _structValue.getType();
        auto astStructTypeNode = gilStructType.getType();

        assert(astStructTypeNode && "Underlying AST TypeBase node is null");
        auto astStructType
            = llvm::cast<glu::types::StructTy>(astStructTypeNode);

        assert(
            index < astStructType->getFieldCount()
            && "Field index out of bounds for "
               "StructDestructureInst::getResultType"
        );
        glu::types::Field const &astField = astStructType->getField(index);
        glu::types::TypeBase *astFieldType = astField.type;

        // TODO: Determine actual size, alignment, and constness for
        // astFieldType. This typically requires a TypeTranslator or DataLayout
        // information from a context (e.g., Module). Using placeholder values
        // for now as this context is not directly available here.
        unsigned placeholderSize = 1; // Placeholder, assuming minimum 1 byte
        unsigned placeholderAlignment
            = 1; // Placeholder, assuming minimum 1 byte alignment
        bool placeholderIsConst = false; // Placeholder

        return Type(
            placeholderSize, placeholderAlignment, placeholderIsConst,
            astFieldType
        );
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
            glu::types::Field const &astField = astStructType->getField(i);
            // getResultType(i) will provide the gil::Type for the field,
            // including the placeholder size/alignment logic for now.
            Type fieldGilType = getResultType(i);
            membersVec.emplace_back(astField.name, fieldGilType, gilStructType);
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
