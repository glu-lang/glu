#ifndef GLU_GIL_GILPRINTER_HPP
#define GLU_GIL_GILPRINTER_HPP

#include "Basic/SourceManager.hpp"
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
    SourceManager *sm;
    llvm::raw_ostream &out;

    bool indentInstructions = false;

public:
    GILPrinter(
        SourceManager *sm = nullptr, llvm::raw_ostream &out = llvm::outs()
    )
        : sm(sm), out(out)
    {
    }

    ~GILPrinter() = default;

    void beforeVisitFunction(Function *fn);
    void afterVisitFunction(Function *fn);
    void beforeVisitBasicBlock(BasicBlock *bb);

    void beforeVisitInst(InstBase *inst);
    void afterVisitInst(InstBase *inst);
    void visitInstBase(InstBase *inst);
    void visitDebugInst(DebugInst *inst);

    void printOperand(Operand op);
    void printOperands(InstBase *inst);
    void printValue(Value val, bool type = true);
    void printLabel(BasicBlock *bb);
    void printSourceLocation(SourceLocation loc);
};
} // namespace glu::gil

#endif // GLU_GIL_GILPRINTER_HPP
