#include "Basic/Diagnostic.hpp"
#include "Basic/SourceLocation.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Instructions.hpp"
#include "GIL/Module.hpp"
#include "PassManager.hpp"

#include <iostream>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::gil {
class BrInst;
class CondBrInst;
}

namespace glu::optimizer {

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
        std::cout << "    Analyzing basic block: " << bb->getLabel().str()
                  << std::endl;

        for (auto &inst : bb->getInstructions()) {
            if (auto *store = llvm::dyn_cast<gil::StoreInst>(&inst)) {
                gil::Value destPtr = store->getDest();
                auto prevStateIt = state.find(destPtr);
                MemoryState prevState = prevStateIt != state.end()
                    ? prevStateIt->second
                    : MemoryState::Uninitialized;

                state[destPtr] = MemoryState::Initialized;
                std::cout
                    << "      Found store - marking location as initialized"
                    << std::endl;
            } else if (auto *load = llvm::dyn_cast<gil::LoadInst>(&inst)) {
                gil::Value srcPtr = load->getValue();
                std::cout
                    << "      Found load - checking if location was initialized"
                    << std::endl;
                auto it = state.find(srcPtr);
                if (it == state.end()
                    || it->second != MemoryState::Initialized) {
                    std::cout << "        WARNING: Loading from potentially "
                                 "uninitialized location"
                              << std::endl;
                }
            } else if (auto *alloca = llvm::dyn_cast<gil::AllocaInst>(&inst)) {
                gil::Value allocatedPtr = alloca->getResult(0);
                state[allocatedPtr] = MemoryState::Uninitialized;
                std::cout
                    << "      Found alloca - marking location as uninitialized"
                    << std::endl;
            } else if (auto *ptrOffsets
                       = llvm::dyn_cast<gil::PtrOffsetInst>(&inst)) {
                gil::Value basePtr = ptrOffsets->getBasePointer();

                auto baseStateIt = state.find(basePtr);
                MemoryState baseState = baseStateIt != state.end()
                    ? baseStateIt->second
                    : MemoryState::Uninitialized;

                state[ptrOffsets->getResult(0)] = baseState;
            } else if (auto *structFieldPtr
                       = llvm::dyn_cast<gil::StructFieldPtrInst>(&inst)) {
                gil::Value basePtr = structFieldPtr->getResult(0);

                auto baseStateIt = state.find(basePtr);
                MemoryState baseState = baseStateIt != state.end()
                    ? baseStateIt->second
                    : MemoryState::Uninitialized;

                state[structFieldPtr->getResult(0)] = baseState;
            }
        }
    }

    void traversePredecessorsForState(gil::BasicBlock *bb)
    {
        auto preds = getPredecessors(bb);
        std::cout << "  Traversing predecessors to check initialization state:"
                  << std::endl;

        if (preds.empty()) {
            std::cout << "    Entry block - all values start uninitialized"
                      << std::endl;
            return;
        }

        for (auto *pred : preds) {
            std::cout << "    Checking predecessor: " << pred->getLabel().str()
                      << std::endl;

            auto predIt = blockEndStates.find(pred);
            if (predIt == blockEndStates.end()) {
                llvm::DenseMap<gil::Value, MemoryState> predState;
                analyzeBasicBlockState(pred, predState);
                blockEndStates[pred] = predState;
                predIt = blockEndStates.find(pred);
            }

            auto const &predState = predIt->second;
            std::cout << "      Predecessor state contains " << predState.size()
                      << " memory locations" << std::endl;

            for (auto const &entry : predState) {
                std::string stateStr
                    = (entry.second == MemoryState::Initialized)
                    ? "initialized"
                    : (entry.second == MemoryState::MaybeInitialized
                           ? "maybe-initialized"
                           : "uninitialized");
                std::cout << "        Memory location: " << stateStr
                          << std::endl;
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

        // Initialize all block states to empty first
        blockEndStates.clear();
        currentState.clear();

        // Perform fixed-point iteration until states stabilize
        bool changed = true;
        int iteration = 0;
        while (changed && iteration < 100) { // Prevent infinite loops
            changed = false;
            iteration++;

            std::cout << "\n=== ITERATION " << iteration << " ===" << std::endl;

            for (auto &bb : func->getBasicBlocks()) {
                // Save the old state for this block
                auto oldState = blockEndStates.find(&bb) != blockEndStates.end()
                    ? blockEndStates[&bb]
                    : llvm::DenseMap<gil::Value, MemoryState> {};

                // Merge states from predecessors
                mergeStatesFromPredecessors(&bb);

                // Analyze this block with the merged state
                analyzeBasicBlockState(&bb, currentState);

                // Check if the end state for this block changed
                if (blockEndStates[&bb] != oldState) {
                    changed = true;
                    std::cout << "Block " << bb.getLabel().str()
                              << " state changed" << std::endl;
                }

                // Update the end state for this block
                blockEndStates[&bb] = currentState;
            }
        }

        if (iteration >= 100) {
            std::cout << "WARNING: Fixed-point iteration did not converge!"
                      << std::endl;
        }
    }

    void beforeVisitBasicBlock(gil::BasicBlock *bb)
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "FINAL VISIT BASIC BLOCK: " << bb->getLabel().str()
                  << std::endl;
        std::cout << "========================================" << std::endl;

        // Use the pre-computed state from fixed-point iteration
        auto it = blockEndStates.find(bb);
        if (it != blockEndStates.end()) {
            // Start with the state at the beginning of this block
            mergeStatesFromPredecessors(bb);
        } else {
            // Entry block case
            currentState.clear();
        }

        std::cout << "Starting state for this block:" << std::endl;
        for (auto const &entry : currentState) {
            printVariableState(entry.first, entry.second, "  ");
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    void afterVisitBasicBlock(gil::BasicBlock *bb)
    {
        blockEndStates[bb] = currentState;
        std::cout << "Ending state for this block:" << std::endl;
        for (auto const &entry : currentState) {
            printVariableState(entry.first, entry.second, "  ");
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    void visitStoreInst(gil::StoreInst *store)
    {
        gil::Value destPtr = store->getDest();

        auto it = currentState.find(destPtr);
        MemoryState prevState = (it != currentState.end())
            ? it->second
            : MemoryState::Uninitialized;

        std::cout << "Found store instruction to memory location:" << std::endl;

        bool warnOnUncertainSet = (prevState == MemoryState::MaybeInitialized);

        if (prevState == MemoryState::Uninitialized) {
            store->setOwnershipKind(gil::StoreOwnershipKind::Init);
            std::cout
                << "  Setting ownership to Init (location was uninitialized)"
                << std::endl;
        } else {
            store->setOwnershipKind(gil::StoreOwnershipKind::Set);
            if (prevState == MemoryState::Initialized) {
                std::cout
                    << "  Setting ownership to Set (location was initialized)"
                    << std::endl;
            } else {
                std::cout << "  Setting ownership to Set (location state is "
                             "uncertain)"
                          << std::endl;
            }
        }

        if (warnOnUncertainSet) {
            std::cout << "  ERROR: Store performed as Set but prior state is "
                         "only maybe-initialized"
                      << std::endl;
            diagManager.warning(
                store->getLocation(),
                "Store to memory location with uncertain initialization"
            );
        }

        currentState[destPtr] = MemoryState::Initialized;
        std::cout << "  Location is now initialized" << std::endl;
    }

    void visitLoadInst(gil::LoadInst *load)
    {
        gil::Value srcPtr = load->getValue();

        auto it = currentState.find(srcPtr);
        MemoryState state = (it != currentState.end())
            ? it->second
            : MemoryState::Initialized; // Default to initialized for
                                        // unknown values

        // Show which memory location we're loading from
        if (auto *defInst = srcPtr.getDefiningInstruction()) {
            std::cout << "Found load from memory location (result "
                      << srcPtr.getIndex() << " of instruction):" << std::endl;
        } else {
            std::cout << "Found load from memory location (block argument "
                      << srcPtr.getIndex() << "):" << std::endl;
        }

        if (state != MemoryState::Initialized) {
            std::cout << "  ERROR: Load from uninitialized memory location"
                      << std::endl;
            // show error with diagnostic manager
            diagManager.warning(
                load->getLocation(), "Load from uninitialized memory location"
            );
        } else {
            std::cout << "  OK: Load from initialized memory location"
                      << std::endl;
        }

        if (load->getOwnershipKind() == gil::LoadOwnershipKind::Take) {
            currentState[srcPtr] = MemoryState::Uninitialized;
            std::cout << "  Load is Take - marking location as uninitialized "
                         "after load"
                      << std::endl;
        }
    }

    void visitAllocaInst(gil::AllocaInst *alloca)
    {
        if (alloca->getResultCount() > 0) {
            gil::Value allocatedPtr = alloca->getResult(0);
            currentState[allocatedPtr] = MemoryState::Uninitialized;
            std::cout
                << "Found alloca - marking new allocation as uninitialized"
                << std::endl;
        }
    }

    void visitPtrOffsetInst(gil::PtrOffsetInst *inst)
    {
        if (inst->getResultCount() == 0) {
            return;
        }

        gil::Value basePtr = inst->getBasePointer();
        MemoryState baseState = getTrackedStateOrDefault(
            basePtr, currentState, MemoryState::Uninitialized
        );

        gil::Value resultPtr = inst->getResult(0);
        currentState[resultPtr] = baseState;

        std::cout << "Found ptr_offset - propagating state from base pointer"
                  << std::endl;
    }

    void visitStructFieldPtrInst(gil::StructFieldPtrInst *inst)
    {
        if (inst->getResultCount() == 0) {
            return;
        }

        gil::Value basePtr = inst->getStructValue();
        MemoryState baseState = getTrackedStateOrDefault(
            basePtr, currentState, MemoryState::Uninitialized
        );

        gil::Value resultPtr = inst->getResult(0);
        currentState[resultPtr] = baseState;

        std::cout
            << "Found struct_field_ptr - propagating state from base struct"
            << std::endl;
    }

    void visitStructExtractInst(gil::StructExtractInst *inst)
    {
        if (inst->getResultCount() == 0) {
            return;
        }

        gil::Value fieldValue = inst->getResult(0);
        currentState[fieldValue] = MemoryState::Initialized;

        std::cout << "Found struct_extract - marking extracted value as"
                     " initialized"
                  << std::endl;
    }

private:
    void printVariableState(
        gil::Value ptr, MemoryState state, std::string const &indent
    )
    {
        std::string stateStr
            = (state == MemoryState::Initialized
                   ? "INITIALIZED"
                   : (state == MemoryState::MaybeInitialized
                          ? "MAYBE_INITIALIZED"
                          : "UNINITIALIZED"));

        if (auto *defInst = ptr.getDefiningInstruction()) {
            // It's a result of an instruction - try to identify what it is
            auto *parentBlock = defInst->getParent();
            std::string blockLabel
                = parentBlock ? parentBlock->getLabel().str() : "<no-block>";
            void const *instAddr = static_cast<void const *>(defInst);

            if (llvm::isa<gil::AllocaInst>(defInst)) {
                std::cout << indent << "%" << ptr.getIndex() << " (alloca"
                          << ", block=" << blockLabel << ", inst=" << instAddr
                          << "): " << stateStr << std::endl;
            } else {
                llvm::StringRef instName = defInst->getInstName();
                std::cout << indent << "%" << ptr.getIndex()
                          << " (instruction result"
                          << ", block=" << blockLabel << ", inst=" << instAddr;
                if (!instName.empty()) {
                    std::cout << ", name=" << instName.str();
                }
                std::cout << "): " << stateStr << std::endl;
            }
        } else {
            // It's a block argument (function parameter)
            auto *defBlock = ptr.getDefiningBlock();
            std::string blockLabel
                = defBlock ? defBlock->getLabel().str() : "<no-block>";
            std::cout << indent << "%" << ptr.getIndex()
                      << " (function parameter"
                      << ", block=" << blockLabel
                      << ", blockPtr=" << static_cast<void const *>(defBlock)
                      << "): " << stateStr << std::endl;
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
            // Entry block - start with empty state
            currentState.clear();
            return;
        }

        llvm::DenseSet<gil::Value> allValues;

        // Collect all values from all predecessors first
        for (auto *pred : preds) {
            auto predIt = blockEndStates.find(pred);
            if (predIt != blockEndStates.end()) {
                for (auto const &entry : predIt->second) {
                    allValues.insert(entry.first);
                }
            }
        }

        // Initialize current state
        currentState.clear();

        // For each value, check consistency across all predecessors
        for (gil::Value value : allValues) {
            bool hasAnyState = false;
            bool missingInPred = false;
            MemoryState mergedState = MemoryState::Uninitialized;

            for (auto *pred : preds) {
                auto predIt = blockEndStates.find(pred);
                if (predIt == blockEndStates.end()) {
                    continue; // No information from this predecessor yet
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
                } else {
                    missingInPred = true;
                }
            }

            if (!hasAnyState) {
                currentState[value] = MemoryState::Uninitialized;
            } else {
                if (missingInPred) {
                    mergedState = mergeMemoryStates(
                        mergedState, MemoryState::Uninitialized
                    );
                }
                currentState[value] = mergedState;
            }
        }
    }

public:
    void afterVisitFunction(gil::Function *func)
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

// load take 면 그 다음에 uninitialzed
// 좋아
// allias structFieldPtn은
// ptr offset => operator []
