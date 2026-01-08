#include "Basic/Diagnostic.hpp"
#include "Basic/SourceLocation.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Instructions.hpp"
#include "GIL/Module.hpp"
#include "PassManager.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::optimizer {

/// @brief GIL optimizer pass that reports loads and stores involving
/// uninitialized memory locations.
///
/// The pass performs a fixed-point data-flow analysis across all basic
/// blocks in a GIL function, tracking a tri-state initialization lattice for
/// each memory value. Diagnostics are emitted when a load observes an
/// uninitialized location or a store that expects an initialized target sees
/// only a maybe-initialized state.
class DetectUninitializedPass
    : public gil::InstVisitor<DetectUninitializedPass> {
private:
    DiagnosticManager &diagManager;

    llvm::DenseMap<gil::BasicBlock *, llvm::SmallVector<gil::BasicBlock *, 4>>
        predecessorMap;

    enum class MemoryState { Uninitialized, MaybeInitialized, Initialized };

    llvm::DenseMap<gil::Value, MemoryState> currentState;

    llvm::DenseMap<gil::BasicBlock *, llvm::DenseMap<gil::Value, MemoryState>>
        blockEndStates;

    static MemoryState mergeMemoryStates(MemoryState lhs, MemoryState rhs)
    {
        if (lhs == rhs) {
            return lhs;
        }

        return MemoryState::MaybeInitialized;
    }

    void buildPredecessorMap(gil::Function *func)
    {
        predecessorMap.clear();

        llvm::DenseSet<gil::BasicBlock *> visited;
        llvm::SmallVector<gil::BasicBlock *, 32> stack;

        if (func->getBasicBlockCount() > 0) {
            stack.push_back(func->getEntryBlock());
        }

        while (!stack.empty()) {
            gil::BasicBlock *currentBB = stack.pop_back_val();

            if (!visited.insert(currentBB).second) {
                continue;
            }

            auto *terminator = currentBB->getTerminator();
            if (!terminator)
                continue;

            llvm::SmallVector<gil::BasicBlock *, 4> successors;

            if (auto *brInst = llvm::dyn_cast<gil::BrInst>(terminator)) {
                if (auto *dest = brInst->getDestination()) {
                    successors.push_back(dest);
                }
            } else if (auto *condBr
                       = llvm::dyn_cast<gil::CondBrInst>(terminator)) {
                if (auto *thenBlock = condBr->getThenBlock()) {
                    successors.push_back(thenBlock);
                }
                if (auto *elseBlock = condBr->getElseBlock()) {
                    successors.push_back(elseBlock);
                }
            }

            for (auto *successor : successors) {
                predecessorMap[successor].push_back(currentBB);

                if (visited.find(successor) == visited.end()) {
                    stack.push_back(successor);
                }
            }
        }
    }

    llvm::ArrayRef<gil::BasicBlock *> getPredecessors(gil::BasicBlock *bb)
    {
        auto it = predecessorMap.find(bb);
        if (it != predecessorMap.end()) {
            return it->second;
        }
        return {};
    }

    void analyzeBasicBlockState(
        gil::BasicBlock *bb, llvm::DenseMap<gil::Value, MemoryState> &state
    )
    {
        for (auto &inst : bb->getInstructions()) {
            if (auto *store = llvm::dyn_cast<gil::StoreInst>(&inst)) {
                state[store->getDest()] = MemoryState::Initialized;
            } else if (auto *alloca = llvm::dyn_cast<gil::AllocaInst>(&inst)) {
                state[alloca->getResult(0)] = MemoryState::Uninitialized;
            } else if (auto *load = llvm::dyn_cast<gil::LoadInst>(&inst)) {
                if (load->getOwnershipKind() == gil::LoadOwnershipKind::Take) {
                    state[load->getValue()] = MemoryState::Uninitialized;
                }
            } else if (auto *drop = llvm::dyn_cast<gil::DropInst>(&inst)) {
                state[drop->getValue()] = MemoryState::Uninitialized;
            } else if (auto *ptrOffsets
                       = llvm::dyn_cast<gil::PtrOffsetInst>(&inst)) {
                state[ptrOffsets->getResult(0)]
                    = state.lookup(ptrOffsets->getBasePtr());
            } else if (auto *structFieldPtr
                       = llvm::dyn_cast<gil::StructFieldPtrInst>(&inst)) {
                state[structFieldPtr->getResult(0)]
                    = state.lookup(structFieldPtr->getStructPtr());
            } else if (auto *bitcastInst
                       = llvm::dyn_cast<gil::BitcastInst>(&inst)) {
                auto source = bitcastInst->getOperand();
                auto result = bitcastInst->getResult(0);

                if (llvm::isa<glu::types::PointerTy>(&*source.getType())
                    && llvm::isa<glu::types::PointerTy>(&*result.getType())) {
                    state[result] = state.lookup(source);
                }
            }
        }
    }

    static MemoryState getTrackedStateOrDefault(
        gil::Value value,
        llvm::DenseMap<gil::Value, MemoryState> const &stateMap,
        MemoryState defaultState
    )
    {
        auto it = stateMap.find(value);
        if (it != stateMap.end()) {
            return it->second;
        }
        return defaultState;
    }

    void mergeStatesFromPredecessors(gil::BasicBlock *bb)
    {
        auto preds = getPredecessors(bb);
        if (preds.empty()) {
            currentState.clear();
            return;
        }

        llvm::DenseSet<gil::Value> allValues;

        for (auto *pred : preds) {
            auto predIt = blockEndStates.find(pred);
            if (predIt != blockEndStates.end()) {
                for (auto const &entry : predIt->second) {
                    allValues.insert(entry.first);
                }
            }
        }

        currentState.clear();

        for (gil::Value value : allValues) {
            bool hasAnyState = false;
            MemoryState mergedState = MemoryState::Uninitialized;

            for (auto *pred : preds) {
                auto predIt = blockEndStates.find(pred);
                if (predIt == blockEndStates.end()) {
                    continue;
                }

                auto valueIt = predIt->second.find(value);
                if (valueIt != predIt->second.end()) {
                    if (!hasAnyState) {
                        mergedState = valueIt->second;
                        hasAnyState = true;
                    } else {
                        mergedState
                            = mergeMemoryStates(mergedState, valueIt->second);
                    }
                }
            }

            if (!hasAnyState) {
                currentState[value] = MemoryState::Uninitialized;
            } else {
                currentState[value] = mergedState;
            }
        }
    }

public:
    DetectUninitializedPass(DiagnosticManager &diagManager)
        : diagManager(diagManager)
    {
    }

    void beforeVisitFunction(gil::Function *func)
    {
        buildPredecessorMap(func);

        blockEndStates.clear();
        currentState.clear();

        bool changed = true;
        int iteration = 0;
        while (changed && iteration < 100) {
            changed = false;
            iteration++;

            for (auto &bb : func->getBasicBlocks()) {
                auto oldState = blockEndStates.lookup(&bb);

                mergeStatesFromPredecessors(&bb);

                analyzeBasicBlockState(&bb, currentState);

                if (blockEndStates[&bb] != oldState) {
                    changed = true;
                }

                blockEndStates[&bb] = currentState;
            }
        }
    }

    void beforeVisitBasicBlock(gil::BasicBlock *bb)
    {
        auto it = blockEndStates.find(bb);
        if (it != blockEndStates.end()) {
            mergeStatesFromPredecessors(bb);
        } else {
            currentState.clear();
        }
    }

    void afterVisitBasicBlock(gil::BasicBlock *bb)
    {
        blockEndStates[bb] = currentState;
    }

    void visitStoreInst(gil::StoreInst *store)
    {
        gil::Value destPtr = store->getDest();

        MemoryState prevState = currentState.lookup(destPtr);

        if (prevState == MemoryState::Uninitialized) {
            store->setOwnershipKind(gil::StoreOwnershipKind::Init);
        } else {
            store->setOwnershipKind(gil::StoreOwnershipKind::Set);
        }

        if (prevState == MemoryState::MaybeInitialized) {
            diagManager.error(
                store->getLocation(),
                "Store to memory location with uncertain initialization"
            );
        }

        currentState[destPtr] = MemoryState::Initialized;
        if (auto *structFieldPtr
            = llvm::dyn_cast_or_null<gil::StructFieldPtrInst>(
                destPtr.getDefiningInstruction()
            )) {
            gil::Value baseStruct = structFieldPtr->getStructPtr();
            currentState[baseStruct] = MemoryState::Initialized;
        }
    }

    void visitLoadInst(gil::LoadInst *load)
    {
        gil::Value srcPtr = load->getValue();

        MemoryState state = currentState.lookup(srcPtr);
        if (!currentState.contains(srcPtr)) {
            // Default to initialized for untracked values
            state = MemoryState::Initialized;
        }

        if (state != MemoryState::Initialized) {
            diagManager.error(
                load->getLocation(), "Load from uninitialized memory location"
            );
        }

        currentState[load->getResult(0)] = state;

        if (load->getOwnershipKind() == gil::LoadOwnershipKind::Take) {
            currentState[srcPtr] = MemoryState::Uninitialized;
        }
    }

    void visitAllocaInst(gil::AllocaInst *alloca)
    {
        currentState[alloca->getResult(0)] = MemoryState::Uninitialized;
    }

    void visitPtrOffsetInst(gil::PtrOffsetInst *inst)
    {
        currentState[inst->getResult(0)] = getTrackedStateOrDefault(
            inst->getBasePtr(), currentState, MemoryState::Uninitialized
        );
    }

    void visitStructFieldPtrInst(gil::StructFieldPtrInst *inst)
    {
        currentState[inst->getResult(0)] = getTrackedStateOrDefault(
            inst->getStructPtr(), currentState, MemoryState::Uninitialized
        );
    }

    void visitBitcastInst(gil::BitcastInst *inst)
    {
        auto value = inst->getOperand();
        auto result = inst->getResult(0);

        if (!llvm::isa<glu::types::PointerTy>(&*value.getType())
            || !llvm::isa<glu::types::PointerTy>(&*result.getType())) {
            return;
        }

        MemoryState sourceState = getTrackedStateOrDefault(
            value, currentState, MemoryState::Uninitialized
        );

        currentState[result] = sourceState;
    }

    void visitStructExtractInst(gil::StructExtractInst *inst)
    {
        currentState[inst->getResult(0)] = MemoryState::Initialized;
    }

    void visitDropInst(gil::DropInst *drop)
    {
        gil::Value srcPtr = drop->getValue();

        MemoryState state = currentState.lookup(srcPtr);
        if (!currentState.contains(srcPtr)) {
            // Default to initialized for untracked values
            state = MemoryState::Initialized;
        }

        if (state != MemoryState::Initialized) {
            diagManager.error(
                drop->getLocation(), "Drop from uninitialized memory location"
            );
        }

        // Drop takes ownership from the pointer, making it uninitialized
        currentState[srcPtr] = MemoryState::Uninitialized;
    }

    void afterVisitFunction([[maybe_unused]] gil::Function *func)
    {
        predecessorMap.clear();
        blockEndStates.clear();
        currentState.clear();
    }
};

void PassManager::runGILDetectUninitializedPass()
{
    DetectUninitializedPass pass(_diagManager);
    pass.visit(_module);
}

} // namespace glu::optimizer
