#include "GILPrinter.hpp"
#include "TypePrinter.hpp"

#include "InstVisitor.hpp"
#include "Instructions.hpp"

#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/WithColor.h>

namespace glu::gil {

struct GILNumberer final : public InstVisitor<GILNumberer> {
    llvm::DenseMap<Value, size_t> valueNumbers;
    llvm::DenseMap<BasicBlock *, size_t> blockNumbers;

    void beforeVisitFunction([[maybe_unused]] Function *fn)
    {
        valueNumbers.clear();
        blockNumbers.clear();
    }

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

    void beforeVisitGlobal(Global *global)
    {
        // Print global header
        llvm::WithColor(out, llvm::raw_ostream::MAGENTA) << "gil_global ";
        llvm::WithColor(out, llvm::raw_ostream::BLUE)
            << "@" << global->getName();
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

    void beforeVisitFunction(Function *fn)
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

    void afterVisitFunction([[maybe_unused]] Function *fn)
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

    void beforeVisitInst([[maybe_unused]] InstBase *inst)
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

    void afterVisitInst([[maybe_unused]] InstBase *inst)
    {
        printSourceLocation(inst->getLocation());
        out << "\n";
    }

    void visitInstBase(InstBase *inst) { printOperands(inst); }

    void visitDebugInst(DebugInst *inst)
    {
        printOperands(inst);
        out << ", ";
        llvm::WithColor(out, llvm::raw_ostream::MAGENTA)
            << inst->getBindingType();
        llvm::WithColor(out, llvm::raw_ostream::BLUE)
            << " \"" << inst->getName() << "\"";
    }

    void visitStoreInst(StoreInst *inst)
    {
        switch (inst->getOwnershipKind()) {
        case StoreOwnershipKind::None: break;
        case StoreOwnershipKind::Init: out << " [init]"; break;
        case StoreOwnershipKind::Set: out << " [set]"; break;
        case StoreOwnershipKind::Trivial: out << " [trivial]"; break;
        }
        printOperands(inst);
    }

    void visitLoadInst(LoadInst *inst)
    {
        switch (inst->getOwnershipKind()) {
        case LoadOwnershipKind::None: break;
        case LoadOwnershipKind::Take: out << " [take]"; break;
        case LoadOwnershipKind::Copy: out << " [copy]"; break;
        case LoadOwnershipKind::Trivial: out << " [trivial]"; break;
        }
        printOperands(inst);
    }

    void printOperand(Value val) { printValue(val); }

    void printOperand(BasicBlock *bb) { printLabel(bb); }

    void printOperand(llvm::APInt intLit)
    {
        llvm::WithColor color(out, llvm::raw_ostream::RED);
        out << intLit;
    }

    void printOperand(llvm::APFloat floatLit)
    {
        llvm::WithColor color(out, llvm::raw_ostream::RED);
        llvm::SmallVector<char, 16> s;
        floatLit.toString(s);
        out << s;
    }

    void printOperand(llvm::StringRef s)
    {
        llvm::WithColor color(out, llvm::raw_ostream::RED);
        out << "\"";
        llvm::printEscapedString(s, out);
        out << "\"";
    }

    void printOperand(Function *fn)
    {
        llvm::WithColor color(out, llvm::raw_ostream::BLUE);
        out << "@";
        out << fn->getName();
    }

    void printOperand(Global *global)
    {
        llvm::WithColor color(out, llvm::raw_ostream::BLUE);
        out << "@";
        out << global->getName();
    }

    void printOperand(Type type)
    {
        llvm::WithColor(out, llvm::raw_ostream::GREEN) << "$";
        printType(&*type);
    }

    void printOperand(Member member)
    {
        out << "#";
        printType(member.getParent().getType());
        out << "::";
        out << member.getName();
    }

    // Overload for CallInst function operand
    void printOperand(std::variant<Value, Function *> function)
    {
        if (std::holds_alternative<Value>(function)) {
            printValue(std::get<Value>(function));
        } else {
            printOperand(std::get<Function *>(function));
        }
    }

    template <typename OperandType>
    void handleOperand(OperandType operand, bool &isFirst)
    {
        if (!isFirst) {
            out << ",";
        }
        out << " ";
        printOperand(std::move(operand));
        isFirst = false;
    }

    template <typename OperandType>
    void handleOperandList(llvm::ArrayRef<OperandType> operand, bool &isFirst)
    {
        for (auto &op : operand) {
            handleOperand(op, isFirst);
        }
    }

    void printOperands(InstBase *inst)
    {
        struct GILOperandPrinter : InstVisitor<GILOperandPrinter> {
            GILPrinter *parent;
            bool isFirst = true;
            GILOperandPrinter(GILPrinter *parent) : parent(parent) { }
#define GIL_OPERAND(Name) parent->handleOperand(inst->get##Name(), isFirst)
#define GIL_OPERAND_LIST(Name)                            \
    parent->handleOperandList(inst->get##Name(), isFirst)
#define GIL_INSTRUCTION_(CLS, Name, Super, Result, ...)          \
    void visit##CLS([[maybe_unused]] CLS *inst) { __VA_ARGS__; }
#include "InstKind.def"
        } printer { this };
        printer.visit(inst);
    }

    void printValue(Value val, bool type = true)
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

    void printLabel(BasicBlock *bb)
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

    void printSourceLocation(SourceLocation loc)
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

    void printType(types::TypeBase *type)
    {
        if (type) {
            llvm::WithColor(out, llvm::raw_ostream::GREEN)
                << ast::TypePrinter().visit(type);
        } else {
            out << "<unknown>";
        }
    }
};

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

// External interface

void printModule(Module *mod, llvm::raw_ostream &out, SourceManager *sm)
{
    GILPrinter(sm, out).visit(mod);
}

void printFunction(Function *fn, llvm::raw_ostream &out, SourceManager *sm)
{
    GILPrinter(sm, out).visit(fn);
}

} // end namespace glu::gil
