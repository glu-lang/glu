#ifndef GLU_GIL_INSTVISITOR_HPP
#define GLU_GIL_INSTVISITOR_HPP

#include "Function.hpp"
#include "Instructions.hpp"
#include "Module.hpp"

namespace glu::gil {

template <typename Impl, typename RetTy = void, typename... ArgTys>
class InstVisitor {
    Impl *asImpl() { return static_cast<Impl *>(this); }

#define CALLBACKS(LONG, SHORT)                         \
    struct SHORT##Callbacks {                          \
        Impl *visitor;                                 \
        LONG *content;                                 \
                                                       \
        SHORT##Callbacks(Impl *visitor, LONG *content) \
            : visitor(visitor), content(content)       \
        {                                              \
            visitor->beforeVisit##SHORT(content);      \
        }                                              \
        ~SHORT##Callbacks()                            \
        {                                              \
            visitor->afterVisit##SHORT(content);       \
        }                                              \
    };
    CALLBACKS(Module, Module)
    CALLBACKS(Function, Function)
    CALLBACKS(BasicBlock, BasicBlock)
    CALLBACKS(InstBase, Inst)
#undef CALLBACKS

public:
    /// @brief Visit an instruction.
    ///
    /// This function dispatches to the visit method for the instruction's
    /// kind.
    ///
    /// @param inst The instruction to visit.
    /// @param ...args Additional arguments to pass to the visitor.
    /// @return The value returned by the visitor.
    RetTy visit(InstBase *inst, ArgTys... args)
    {
        return _visitInst(inst, std::forward<ArgTys>(args)...);
    }

    /// @brief Visit a function, its basic blocks, and instructions.
    ///
    /// This function dispatches to the visit method for the function's
    /// content.
    ///
    /// @param fn The function to visit.
    /// @param ...args Additional arguments to pass to the visitor.
    void visit(Function *inst, ArgTys... args)
    {
        _visitFunction(inst, std::forward<ArgTys>(args)...);
    }

    /// @brief Visit a module, its functions, and all basic blocks and
    /// instructions.
    ///
    /// This function dispatches to the visit method for the module's
    /// content.
    ///
    /// @param mod The module to visit.
    /// @param ...args Additional arguments to pass to the visitor.
    void visit(Module *mod, ArgTys... args)
    {
        _visitModule(mod, std::forward<ArgTys>(args)...);
    }

    void _visitModule(Module *mod, ArgTys... args)
    {
        ModuleCallbacks callbacks(asImpl(), mod);
        for (auto &fn : mod->getFunctions()) {
            _visitFunction(&fn, std::forward<ArgTys>(args)...);
        }
    }

    void _visitFunction(Function *fn, ArgTys... args)
    {
        FunctionCallbacks callbacks(asImpl(), fn);
        for (auto &bb : fn->getBasicBlocks()) {
            _visitBasicBlock(&bb, std::forward<ArgTys>(args)...);
        }
    }

    void _visitBasicBlock(BasicBlock *bb, ArgTys... args)
    {
        BasicBlockCallbacks callbacks(asImpl(), bb);
        for (auto &inst : bb->getInstructions()) {
            visit(&inst, std::forward<ArgTys>(args)...);
        }
    }

    RetTy _visitInst(InstBase *inst, ArgTys... args)
    {
        InstCallbacks callbacks(asImpl(), inst);
        switch (inst->getKind()) {
#define GIL_INSTRUCTION(CLS, NAME, PARENT)                   \
case InstKind::CLS##Kind:                                    \
    return asImpl()->visit##CLS(                             \
        llvm::cast<CLS>(inst), std::forward<ArgTys>(args)... \
    );
#include "InstKind.def"
        default: llvm_unreachable("Unknown instruction kind");
        }
    }

    /// @brief An action to run before visiting a module.
    /// @param mod the module about to be visited
    void beforeVisitModule([[maybe_unused]] Module *mod) { }
    /// @brief An action to run after visiting a module.
    /// @param fn the module that was just visited
    void afterVisitModule([[maybe_unused]] Module *mod) { }

    /// @brief An action to run before visiting a function.
    /// @param fn the function about to be visited
    void beforeVisitFunction([[maybe_unused]] Function *fn) { }
    /// @brief An action to run after visiting a function.
    /// @param fn the function that was just visited
    void afterVisitFunction([[maybe_unused]] Function *fn) { }

    /// @brief An action to run before visiting a basic block.
    /// @param bb the basic block about to be visited
    void beforeVisitBasicBlock([[maybe_unused]] BasicBlock *bb) { }
    /// @brief An action to run after visiting a basic block.
    /// @param bb the basic block that was just visited
    void afterVisitBasicBlock([[maybe_unused]] BasicBlock *bb) { }

    /// @brief An action to run before visiting any instruction.
    /// @param inst the instruction about to be visited
    void beforeVisitInst([[maybe_unused]] InstBase *inst) { }
    /// @brief An action to run after visiting any instruction.
    /// @param inst the instruction that was just visited
    void afterVisitInst([[maybe_unused]] InstBase *inst) { }

    RetTy visitInstBase(
        [[maybe_unused]] InstBase *inst, [[maybe_unused]] ArgTys... args
    )
    {
    }

#define GIL_INSTRUCTION(CLS, NAME, PARENT)                                   \
    RetTy visit##CLS(CLS *inst, ArgTys... args)                              \
    {                                                                        \
        return asImpl()->visit##PARENT(inst, std::forward<ArgTys>(args)...); \
    }
#define GIL_INSTRUCTION_SUPER(CLS, PARENT) GIL_INSTRUCTION(CLS, "", PARENT)
#include "InstKind.def"
};

} // namespace glu::gil

#endif // GLU_GIL_INSTVISITOR_HPP
