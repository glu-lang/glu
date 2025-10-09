#include "GILPrinter.hpp"
#include "TypePrinter.hpp"

#include <llvm/Support/WithColor.h>

namespace glu::gil {

void GILNumberer::beforeVisitFunction([[maybe_unused]] Function *fn)
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

void GILPrinter::beforeVisitGlobal(Global *global)
{
    // Print global header
    llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << "gil_global ";
    llvm::WithColor(out, llvm::raw_ostream::BLUE) << "@" << global->getName();
    out << " : ";
    llvm::WithColor(out, llvm::raw_ostream::GREEN) << "$";
    printType(global->getType());
    if (global->getInitializer()) {
        out << " = ";
        llvm::WithColor(out, llvm::raw_ostream::BLUE)
            << "@" << global->getInitializer()->getName();
    } else if (global->hasInitializer()) {
        out << " = ";
        llvm::WithColor(out, llvm::raw_ostream::BLUE) << "<external>";
    }
    out << ";\n\n";
}

void GILPrinter::beforeVisitFunction(Function *fn)
{
    // Calculate value numbers
    numberer.visit(fn);
    // Print function header
    llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << "gil ";
    llvm::WithColor(out, llvm::raw_ostream::BLUE) << "@" << fn->getName();
    out << " : ";
    llvm::WithColor(out, llvm::raw_ostream::GREEN) << "$";
    printType(fn->getType());
    out << " {\n";
    indentInstructions = true;
}

void GILPrinter::afterVisitFunction([[maybe_unused]] Function *fn)
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

void GILPrinter::beforeVisitInst([[maybe_unused]] InstBase *inst)
{
    if (indentInstructions) {
        out << "    ";
    }
    if (size_t results = inst->getResultCount()) {
        for (size_t i = 0; i < results; ++i) {
            printValue(inst->getResult(i), false);
            if (i + 1 < inst->getResultCount()) {
                out << ", ";
            }
        }
        out << " = ";
    }
    llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << inst->getInstName();
}

void GILPrinter::afterVisitInst([[maybe_unused]] InstBase *inst)
{
    printSourceLocation(inst->getLocation());
    out << "\n";
}

void GILPrinter::visitInstBase(InstBase *inst)
{
    printOperands(inst);
}

void GILPrinter::printOperand(Operand op)
{
    switch (op.getKind()) {
    case OperandKind::ValueKind: printValue(op.getValue()); break;
    case OperandKind::LiteralIntKind: {
        llvm::WithColor color(out, llvm::raw_ostream::RED);
        out << op.getLiteralInt();
        break;
    }
    case OperandKind::LiteralFloatKind: {
        llvm::WithColor color(out, llvm::raw_ostream::RED);
        llvm::SmallVector<char, 16> s;
        op.getLiteralFloat().toString(s);
        out << s;
        break;
    }
    case OperandKind::LiteralStringKind: {
        llvm::WithColor color(out, llvm::raw_ostream::RED);
        out << "\"";
        llvm::printEscapedString(op.getLiteralString(), out);
        out << "\"";
        break;
    }
    case OperandKind::SymbolKind: {
        llvm::WithColor color(out, llvm::raw_ostream::BLUE);
        out << "@";
        out << op.getSymbol()->getName();
        break;
    }
    case OperandKind::GlobalKind: {
        llvm::WithColor color(out, llvm::raw_ostream::BLUE);
        out << "@";
        out << op.getGlobal()->getName();
        break;
    }
    case OperandKind::TypeKind:
        llvm::WithColor(out, llvm::raw_ostream::GREEN) << "$";
        printType(&*op.getType());
        break;
    case OperandKind::MemberKind:
        out << "#";
        printType(op.getMember().getParent().getType());
        out << "::";
        out << op.getMember().getName();
        break;
    case OperandKind::LabelKind: printLabel(op.getLabel()); break;
    }
}

void GILPrinter::printOperands(InstBase *inst)
{
    size_t operands = inst->getOperandCount();
    for (size_t i = 0; i < operands; ++i) {
        if (i != 0)
            out << ",";
        out << " ";
        printOperand(inst->getOperand(i));
    }
}

void GILPrinter::printValue(Value val, bool type)
{
    {
        llvm::WithColor cyan(out, llvm::raw_ostream::CYAN);
        out << "%";
        if (val == Value::getEmptyKey()) {
            out << "<empty>";
            return;
        }
        if (numberer.valueNumbers.contains(val))
            out << numberer.valueNumbers[val];
        else
            out << "<unknown>"; // TODO: more info?
    }
    if (type) {
        out << " : ";
        llvm::WithColor(out, llvm::raw_ostream::GREEN) << "$";
        printType(&*val.getType());
    }
}

void GILPrinter::printLabel(BasicBlock *bb)
{
    llvm::WithColor color(out, llvm::raw_ostream::YELLOW);
    if (bb->getLabel().empty()) {
        if (numberer.blockNumbers.contains(bb))
            out << "bb" << numberer.blockNumbers[bb];
        else
            out << "bb<unknown>";
    } else {
        out << bb->getLabel();
    }
}

void GILPrinter::printSourceLocation(SourceLocation loc)
{
    if (!loc.isValid() || !sm)
        return;

    out << ", ";
    llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << "loc ";
    llvm::WithColor(out, llvm::raw_ostream::YELLOW)
        << "\"" << sm->getBufferName(loc)
        << "\":" << sm->getSpellingLineNumber(loc) << ":"
        << sm->getSpellingColumnNumber(loc);
}

void GILPrinter::visitDebugInst(DebugInst *inst)
{
    printOperands(inst);
    out << ", ";
    llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << inst->getBindingType();
    llvm::WithColor(out, llvm::raw_ostream::BLUE)
        << " \"" << inst->getName() << "\"";
}

void GILPrinter::printType(types::TypeBase *type)
{
    if (type) {
        llvm::WithColor(out, llvm::raw_ostream::GREEN)
            << ast::TypePrinter().visit(type);
    } else {
        out << "<unknown>";
    }
}

// Convenience methods to print directly to stdout
// For use in lldb
// These are designed to be resilient (we might be in an invalid state)

void InstBase::print()
{
    SourceManager *sm = nullptr;
    if (auto *bb = this->getParent()) {
        if (auto *func = bb->getParent()) {
            if (auto *decl = func->getDecl()) {
                if (auto *mod = decl->getModule()) {
                    sm = mod->getSourceManager();
                }
            }
        }
    }
    GILPrinter(sm).visit(this);
}

void Function::print()
{
    SourceManager *sm = nullptr;
    if (auto *decl = this->getDecl()) {
        if (auto *mod = decl->getModule()) {
            sm = mod->getSourceManager();
        }
    }
    GILPrinter(sm).visit(this);
}

void Global::print()
{
    SourceManager *sm = nullptr;
    if (auto *decl = this->getDecl()) {
        if (auto *mod = decl->getModule()) {
            sm = mod->getSourceManager();
        }
    }
    GILPrinter(sm).visit(this);
}

void Module::print()
{
    SourceManager *sm = nullptr;
    for (auto &fn : this->getFunctions()) {
        if (fn.getDecl() == nullptr) {
            continue;
        }
        if (auto *mod = fn.getDecl()->getModule()) {
            sm = mod->getSourceManager();
            break;
        }
    }
    GILPrinter(sm).visit(this);
}

} // end namespace glu::gil
