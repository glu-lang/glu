#include "InstVisitor.hpp"
#include "Instructions.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::gil {

struct GILNumberer final : public InstVisitor<GILNumberer> {
    llvm::DenseMap<Value, size_t> valueNumbers;
    llvm::DenseMap<BasicBlock *, size_t> blockNumbers;

    void visitInstBase(InstBase *inst)
    {
        size_t results = inst->getResultCount();
        for (size_t i = 0; i < results; ++i) {
            valueNumbers[inst->getResult(i)] = valueNumbers.size();
        }
    }

    void beforeVisitBasicBlock(BasicBlock *bb)
    {
        size_t args = bb->getArgumentCount();
        for (size_t i = 0; i < args; ++i) {
            valueNumbers[bb->getArgument(i)] = valueNumbers.size();
        }
        blockNumbers[bb] = blockNumbers.size();
    }
};

class GILPrinter : public InstVisitor<GILPrinter> {
    llvm::DenseMap<Value, size_t> valueNumbers;
    llvm::DenseMap<BasicBlock *, size_t> blockNumbers;
    llvm::raw_ostream &out;

    bool indentInstructions = false;

public:
    GILPrinter(llvm::raw_ostream &out = llvm::outs()) : out(out) { }
    ~GILPrinter() = default;

    void beforeVisitFunction(Function *fn)
    {
        // Calculate value numbers
        GILNumberer numberer;
        numberer.visit(fn);
        std::swap(valueNumbers, numberer.valueNumbers);
        std::swap(blockNumbers, numberer.blockNumbers);
        // Print function header
        out << "gil @" << fn->getName() << " : $";
        // TODO: visitType
        out << " {\n";
        indentInstructions = true;
    }

    void afterVisitFunction(Function *fn)
    {
        indentInstructions = false;
        out << "}\n\n";
    }

    void beforeVisitBasicBlock(BasicBlock *bb)
    {
        printLabel(bb);
        if (size_t argcount = bb->getArgumentCount()) {
            out << "(";
            for (size_t i = 0; i < argcount; ++i) {
                if (i != 0) {
                    out << ", ";
                }
                printValue(bb->getArgument(i));
            }
            out << ")";
        }
        out << ":\n";
    }

    void beforeVisitInst(InstBase *inst)
    {
        if (indentInstructions) {
            out << "    ";
        }
    }

    void afterVisitInst(InstBase *inst) { out << "\n"; }

    void visitInstBase(InstBase *inst)
    {
        if (size_t results = inst->getResultCount()) {
            for (size_t i = 0; i < results; ++i) {
                printValue(inst->getResult(i));
                if (i + 1 < inst->getResultCount()) {
                    out << ", ";
                }
            }
            out << " = ";
        }
        out << inst->getInstName();
        size_t operands = inst->getOperandCount();
        for (size_t i = 0; i < operands; ++i) {
            if (i != 0)
                out << ",";
            out << " ";
            printOperand(inst->getOperand(i));
        }
    }

    void printOperand(Operand op)
    {
        switch (op.getKind()) {
        case OperandKind::ValueKind: printValue(op.getValue()); break;
        case OperandKind::LiteralIntKind: out << op.getLiteralInt(); break;
        case OperandKind::LiteralFloatKind:
            op.getLiteralFloat().print(out);
            break;
        case OperandKind::LiteralStringKind:
            out << "\"";
            llvm::printEscapedString(op.getLiteralString(), out);
            out << "\"";
            break;
        case OperandKind::SymbolKind:
            out << "@";
            out << op.getSymbol()->getName();
            break;
        case OperandKind::TypeKind:
            out << "$";
            // TODO: visitType
            break;
        case OperandKind::MemberKind:
            out << "#";
            // TODO
            break;
        case OperandKind::LabelKind: printLabel(op.getLabel()); break;
        }
    }

    void printValue(Value val)
    {
        out << "%";
        if (valueNumbers.contains(val))
            out << valueNumbers[val];
        else
            out << "<unknown>"; // TODO: more info?
    }

    void printLabel(BasicBlock *bb)
    {
        if (bb->getLabel().empty()) {
            if (blockNumbers.contains(bb))
                out << "bb" << blockNumbers[bb];
            else
                out << "bb<unknown>";
        } else {
            out << bb->getLabel();
        }
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
