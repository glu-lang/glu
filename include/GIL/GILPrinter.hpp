#include "InstVisitor.hpp"
#include "Instructions.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::gil {

struct GILNumberer final : public InstVisitor<GILNumberer> {
    llvm::DenseMap<Value, size_t> valueNumbers;
    llvm::DenseMap<BasicBlock *, size_t> blockNumbers;

    void beforeVisitFunction(Function *fn);
    void visitInstBase(InstBase *inst);
    void beforeVisitBasicBlock(BasicBlock *bb);
};

class GILPrinter : public InstVisitor<GILPrinter> {
    GILNumberer numberer;
    llvm::raw_ostream &out;

    bool indentInstructions = false;

public:
    GILPrinter(llvm::raw_ostream &out = llvm::outs()) : out(out) { }
    ~GILPrinter() = default;

    void beforeVisitFunction(Function *fn);
    void afterVisitFunction(Function *fn);
    void beforeVisitBasicBlock(BasicBlock *bb);

    void beforeVisitInst(InstBase *inst);
    void afterVisitInst(InstBase *inst);
    void visitInstBase(InstBase *inst);

    void printOperand(Operand op);
    void printValue(Value val);
    void printLabel(BasicBlock *bb);
};
} // namespace glu::gil

// support for Value keys in DenseMap
namespace llvm {
template <> struct DenseMapInfo<glu::gil::Value> {
    static inline glu::gil::Value getEmptyKey()
    {
        return glu::gil::Value::getEmptyKey();
    }

    static inline glu::gil::Value getTombstoneKey()
    {
        return glu::gil::Value::getTombstoneKey();
    }

    static unsigned getHashValue(glu::gil::Value const &val)
    {
        return DenseMapInfo<std::pair<glu::gil::InstBase *, unsigned>>::
            getHashValue(
                std::make_pair(val.getDefiningInstruction(), val.getIndex())
            );
    }

    static bool isEqual(glu::gil::Value const &lhs, glu::gil::Value const &rhs)
    {
        return lhs == rhs;
    }
};
} // namespace llvm
