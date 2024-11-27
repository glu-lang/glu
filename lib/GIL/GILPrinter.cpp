#include "GILPrinter.hpp"

namespace glu::gil {

void GILNumberer::beforeVisitFunction(Function *fn)
{
    valueNumbers.clear();
    blockNumbers.clear();
}

void GILNumberer::beforeVisitBasicBlock(BasicBlock *bb)
{
    size_t args = bb->getArgumentCount();
    for (size_t i = 0; i < args; ++i) {
        valueNumbers[bb->getArgument(i)] = valueNumbers.size();
    }
    blockNumbers[bb] = blockNumbers.size();
}

void GILNumberer::visitInstBase(InstBase *inst)
{
    size_t results = inst->getResultCount();
    for (size_t i = 0; i < results; ++i) {
        valueNumbers[inst->getResult(i)] = valueNumbers.size();
    }
}

void GILPrinter::beforeVisitFunction(Function *fn)
{
    // Calculate value numbers
    numberer.visit(fn);
    // Print function header
    out << "gil @" << fn->getName() << " : $";
    // TODO: visitType
    out << " {\n";
    indentInstructions = true;
}

void GILPrinter::afterVisitFunction(Function *fn)
{
    indentInstructions = false;
    out << "}\n\n";
}

void GILPrinter::beforeVisitBasicBlock(BasicBlock *bb)
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

void GILPrinter::beforeVisitInst(InstBase *inst)
{
    if (indentInstructions) {
        out << "    ";
    }
}

void GILPrinter::afterVisitInst(InstBase *inst)
{
    out << "\n";
}

void GILPrinter::visitInstBase(InstBase *inst)
{
    if (size_t results = inst->getResultCount()) {
        for (size_t i = 0; i < results; ++i) {
            printValue(inst->getResult(i), false);
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

void GILPrinter::printOperand(Operand op)
{
    switch (op.getKind()) {
    case OperandKind::ValueKind: printValue(op.getValue()); break;
    case OperandKind::LiteralIntKind: out << op.getLiteralInt(); break;
    case OperandKind::LiteralFloatKind: {
        llvm::SmallVector<char, 16> s;
        op.getLiteralFloat().toString(s);
        out << s;
        break;
    }
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

void GILPrinter::printValue(Value val, bool type)
{
    out << "%";
    if (numberer.valueNumbers.contains(val))
        out << numberer.valueNumbers[val];
    else
        out << "<unknown>"; // TODO: more info?
    if (type) {
        out << " : $"; // TODO: visitType
    }
}

void GILPrinter::printLabel(BasicBlock *bb)
{
    if (bb->getLabel().empty()) {
        if (numberer.blockNumbers.contains(bb))
            out << "bb" << numberer.blockNumbers[bb];
        else
            out << "bb<unknown>";
    } else {
        out << bb->getLabel();
    }
}

} // end namespace glu::gil
