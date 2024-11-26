#include "InstVisitor.hpp"
#include "Instructions.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::gil {

struct GILNumberer final : public InstVisitor<GILNumberer> {
    llvm::DenseMap<Value, size_t> valueNumbers;

    void visitInstBase(InstBase *inst)
    {
        size_t results = inst->getResultCount();
        for (size_t i = 0; i < results; ++i) {
            valueNumbers[inst->getResult(i)] = valueNumbers.size();
        }
    }
};

class GILPrinter : public InstVisitor<GILPrinter> {
    llvm::DenseMap<Value, size_t> valueNumbers;
    llvm::raw_ostream &out;

public:
    GILPrinter(llvm::raw_ostream &out = llvm::outs()) : out(out) { }
    ~GILPrinter() = default;

    void beforeVisitFunction(Function *fn)
    {
        GILNumberer numberer;
        numberer.visit(fn);
        std::swap(valueNumbers, numberer.valueNumbers);
    }

    void beforeVisitInst(InstBase *inst)
    {
        // indent?
    }

    void afterVisitInst(InstBase *inst) { out << "\n"; }

    void visitInstBase(InstBase *inst)
    {
        if (size_t results = inst->getResultCount()) {
            for (size_t i = 0; i < results; ++i) {
                out << "%";
                out << valueNumbers[inst->getResult(i)];
                if (i + 1 < inst->getResultCount()) {
                    out << ", ";
                }
            }
            out << " = ";
        }
        out << inst->getInstName();
    }
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
