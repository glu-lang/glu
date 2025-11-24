#include "BasicBlock.hpp"
#include "InstVisitor.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::gil {

llvm::StringRef InstBase::getInstName() const
{
    switch (getKind()) {
#define GIL_INSTRUCTION(CLS, NAME, PARENT) \
    case InstKind::CLS##Kind: return NAME;
#include "InstKind.def"
    default: llvm_unreachable("Unknown instruction kind");
    }
}

BasicBlock *Value::getDefiningBlock() const
{
    if (auto block = value.dyn_cast<BasicBlock *>()) {
        return block;
    }
    return llvm::cast<InstBase *>(value)->getParent();
}

struct ValueReplacer : InstVisitor<ValueReplacer> {
    Value oldValue;
    Value newValue;

    template <
        typename ConcreteInstType, typename AccessorInstType,
        typename OperandType>
    void maybeReplaceOperand(
        ConcreteInstType *inst, OperandType (AccessorInstType::*getter)() const,
        void (AccessorInstType::*setter)(OperandType)
    )
    {
        if constexpr (std::is_same_v<OperandType, Value>) {
            if ((inst->*getter)() == oldValue) {
                (inst->*setter)(newValue);
            }
        } else if constexpr (std::is_same_v<
                                 OperandType,
                                 std::variant<Value, Function *>>) {
            auto operand = (inst->*getter)();
            if (std::holds_alternative<Value>(operand)
                && std::get<Value>(operand) == oldValue) {
                (inst->*setter)(newValue);
            }
        }
    }

    template <typename OperandType>
    void maybeReplaceListOperand(llvm::MutableArrayRef<OperandType> list)
    {
        if constexpr (std::is_same_v<OperandType, Value>) {
            for (auto &operand : list) {
                if (operand == oldValue) {
                    operand = newValue;
                }
            }
        }
    }

    ValueReplacer(Value oldValue, Value newValue)
        : oldValue(oldValue), newValue(newValue)
    {
    }
#define GIL_OPERAND(Name)                                          \
    maybeReplaceOperand(                                           \
        inst, &LocalInstType::get##Name, &LocalInstType::set##Name \
    )
#define GIL_OPERAND_LIST(Name)                          \
    maybeReplaceListOperand(inst->get##Name##Mutable())
#define GIL_INSTRUCTION_(CLS, Name, Super, Result, ...) \
    void visit##CLS([[maybe_unused]] CLS *inst)         \
    {                                                   \
        using LocalInstType [[maybe_unused]] = CLS;     \
        __VA_ARGS__;                                    \
    }
#include "InstKind.def"
};

void Value::replaceAllUsesWith(Value newValue)
{
    ValueReplacer replacer { *this, newValue };
    replacer.visit(getDefiningBlock()->getParent());
}

void InstBase::eraseFromParent()
{
    auto *parentBlock = getParent();
    assert(parentBlock && "Instruction has no parent basic block");
    parentBlock->removeInstruction(this);
}

size_t InstBase::getResultCount() const
{
    switch (getKind()) {
#define GIL_RESULT_SINGLE(CLS) 1
#define GIL_RESULT_NONE(CLS) 0
#define GIL_RESULT_MULTIPLE(CLS) llvm::cast<CLS>(this)->getResultCountImpl()
#define GIL_INSTRUCTION_(CLS, NAME, PARENT, RESULT, ...) \
    case InstKind::CLS##Kind: return RESULT(CLS);
#include "InstKind.def"
    default: llvm_unreachable("Unknown instruction kind");
    }
}

Type InstBase::getResultType(size_t index) const
{
    switch (getKind()) {
#define GIL_RESULT_SINGLE(CLS) return llvm::cast<CLS>(this)->getResultType()
#define GIL_RESULT_NONE(CLS) llvm_unreachable("Invalid index")
#define GIL_RESULT_MULTIPLE(CLS)                           \
    return llvm::cast<CLS>(this)->getResultTypeImpl(index)
#define GIL_INSTRUCTION_(CLS, NAME, PARENT, RESULT, ...) \
    case InstKind::CLS##Kind: RESULT(CLS);
#include "InstKind.def"
    default: llvm_unreachable("Unknown instruction kind");
    }
}

} // end namespace glu::gil

namespace llvm {

glu::gil::BasicBlock *ilist_traits<glu::gil::InstBase>::getContainingBlock()
{
    size_t Offset = reinterpret_cast<size_t>(
        &((glu::gil::BasicBlock *) nullptr
              ->*glu::gil::BasicBlock::getSublistAccess(
                  static_cast<glu::gil::InstBase *>(nullptr)
              ))
    );
    iplist<glu::gil::InstBase, ilist_parent<glu::gil::BasicBlock>> *Anchor
        = static_cast<
            iplist<glu::gil::InstBase, ilist_parent<glu::gil::BasicBlock>> *>(
            this
        );
    return reinterpret_cast<glu::gil::BasicBlock *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}

} // end namespace llvm
