#ifndef GLU_GILGEN_GILPASSES_DETECTUNINITIALIZED_HPP
#define GLU_GILGEN_GILPASSES_DETECTUNINITIALIZED_HPP

    #include "Basic/Diagnostic.hpp"
    #include "Basic/SourceLocation.hpp"
    #include "GIL/BasicBlock.hpp"
    #include "GIL/Function.hpp"
    #include "GIL/InstVisitor.hpp"
    #include "GIL/Instructions.hpp"
    #include "GIL/Module.hpp"

    #include <iostream>
    #include <llvm/ADT/DenseMap.h>
    #include <llvm/ADT/DenseSet.h>
    #include <llvm/ADT/SmallVector.h>

namespace glu::gil {
class BrInst;
class CondBrInst;
}

namespace glu::gilgen {

class GILDetectUninitializedPass
    : public gil::InstVisitor<GILDetectUninitializedPass> {
private:
    DiagnosticManager &diagManager;

    llvm::DenseMap<gil::BasicBlock *, llvm::SmallVector<gil::BasicBlock *, 4>>
        predecessorMap;

    enum class MemoryState { Uninitialized, Initialized };

    llvm::DenseMap<gil::Value, MemoryState> currentState;

    llvm::DenseMap<gil::BasicBlock *, llvm::DenseMap<gil::Value, MemoryState>>
        blockEndStates;

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
        // std::cout << "    Analyzing basic block: " << bb->getLabel().str()
        //           << std::endl;

        for (auto &inst : bb->getInstructions()) {
            if (auto *store = llvm::dyn_cast<gil::StoreInst>(&inst)) {
                gil::Value destPtr = store->getDest();
                state[destPtr] = MemoryState::Initialized;
                // std::cout
                //     << "      Found store - marking location as initialized"
                //     << std::endl;
            } else if (auto *load = llvm::dyn_cast<gil::LoadInst>(&inst)) {
                gil::Value srcPtr = load->getValue();
                // std::cout
                //     << "      Found load - checking if location was
                //     initialized"
                //     << std::endl;
                auto it = state.find(srcPtr);
                if (it == state.end()
                    || it->second == MemoryState::Uninitialized) {
                    // std::cout << "        WARNING: Loading from potentially "
                    //              "uninitialized location"
                    //           << std::endl;
                }
            } else if (auto *alloca = llvm::dyn_cast<gil::AllocaInst>(&inst)) {
                gil::Value allocatedPtr = alloca->getResult(0);
                state[allocatedPtr] = MemoryState::Uninitialized;
                // std::cout
                //     << "      Found alloca - marking location as
                //     uninitialized"
                //     << std::endl;
            } else if (auto *ptrOffsets
                       = llvm::dyn_cast<gil::PtrOffsetInst>(&inst)) {
                gil::Value basePtr = ptrOffsets->getBasePointer();

                state[ptrOffsets->getResult(0)] = state[basePtr];
            } else if (auto *structFieldPtr
                       = llvm::dyn_cast<gil::StructFieldPtrInst>(&inst)) {
                gil::Value basePtr = structFieldPtr->getResult(0);

                state[structFieldPtr->getResult(0)] = state[basePtr];
            }
        }
    }

    void traversePredecessorsForState(gil::BasicBlock *bb)
    {
        auto preds = getPredecessors(bb);
        // std::cout << "  Traversing predecessors to check initialization
        // state:"
        //           << std::endl;

        if (preds.empty()) {
            // std::cout << "    Entry block - all values start uninitialized"
            //           << std::endl;
            return;
        }

        for (auto *pred : preds) {
            // std::cout << "    Checking predecessor: " <<
            // pred->getLabel().str()
            //           << std::endl;

            auto predIt = blockEndStates.find(pred);
            if (predIt == blockEndStates.end()) {
                llvm::DenseMap<gil::Value, MemoryState> predState;
                analyzeBasicBlockState(pred, predState);
                blockEndStates[pred] = predState;
                predIt = blockEndStates.find(pred);
            }

            auto const &predState = predIt->second;
            // std::cout << "      Predecessor state contains " <<
            // predState.size()
            //           << " memory locations" << std::endl;

            for (auto const &entry : predState) {
                std::string stateStr
                    = (entry.second == MemoryState::Initialized)
                    ? "initialized"
                    : "uninitialized";
                // std::cout << "        Memory location: " << stateStr
                //           << std::endl;
            }
        }
    }

public:
    GILDetectUninitializedPass(DiagnosticManager &diagManager)
        : diagManager(diagManager)
    {
    }

    void beforeVisitFunction(gil::Function *func)
    {
        buildPredecessorMap(func);

        for (auto &bb : func->getBasicBlocks()) {
            auto preds = getPredecessors(&bb);
            // std::cout << "Basic Block: " << bb.getLabel().str() << std::endl;
            if (preds.empty()) {
                // std::cout << "  No predecessors (entry block)" << std::endl;
            } else {
                // std::cout << "  Predecessors: ";
                for (auto *pred : preds) {
                    // std::cout << pred->getLabel().str() << " ";
                }
                // std::cout << std::endl;

                traversePredecessorsForState(&bb);
            }
        }
    }

    void beforeVisitBasicBlock(gil::BasicBlock *bb)
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "VISITING BASIC BLOCK: " << bb->getLabel().str()
                  << std::endl;
        std::cout << "========================================" << std::endl;

        auto preds = getPredecessors(bb);
        if (preds.empty()) {
            std::cout << "This is the ENTRY block (no predecessors)"
                      << std::endl;
        } else {
            std::cout << "This block has predecessors: ";
            for (auto *pred : preds) {
                std::cout << pred->getLabel().str() << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "\nBEFORE merging - current state has "
                  << currentState.size() << " variables:" << std::endl;
        for (auto const &entry : currentState) {
            printVariableState(entry.first, entry.second, "  ");
        }

        mergeStatesFromPredecessors(bb);

        std::cout << "\nAFTER merging - current state has "
                  << currentState.size() << " variables:" << std::endl;
        for (auto const &entry : currentState) {
            printVariableState(entry.first, entry.second, "  ");
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    void afterVisitBasicBlock(gil::BasicBlock *bb)
    {
        blockEndStates[bb] = currentState;
    }

    void visitStoreInst(gil::StoreInst *store)
    {
        gil::Value destPtr = store->getDest();

        auto it = currentState.find(destPtr);
        MemoryState prevState = (it != currentState.end())
            ? it->second
            : MemoryState::Uninitialized;

        std::cout << "Found store instruction to memory location:" << std::endl;

        if (prevState == MemoryState::Uninitialized) {
            store->setOwnershipKind(gil::StoreOwnershipKind::Init);
            std::cout
                << "  Setting ownership to Init (location was uninitialized)"
                << std::endl;
        } else {
            store->setOwnershipKind(gil::StoreOwnershipKind::Set);
            std::cout << "  Setting ownership to Set (location was initialized)"
                      << std::endl;
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
            : MemoryState::Uninitialized; // Default to uninitialized for
                                          // unknown values

        // Show which memory location we're loading from
        if (auto *defInst = srcPtr.getDefiningInstruction()) {
            std::cout << "Found load from memory location (result "
                      << srcPtr.getIndex() << " of instruction):" << std::endl;
        } else {
            std::cout << "Found load from memory location (block argument "
                      << srcPtr.getIndex() << "):" << std::endl;
        }

        if (state == MemoryState::Uninitialized) {
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

private:
    void printVariableState(
        gil::Value ptr, MemoryState state, std::string const &indent
    )
    {
        std::string stateStr
            = (state == MemoryState::Initialized ? "INITIALIZED"
                                                 : "UNINITIALIZED");

        if (auto *defInst = ptr.getDefiningInstruction()) {
            // It's a result of an instruction - try to identify what it is
            if (llvm::isa<gil::AllocaInst>(defInst)) {
                std::cout << indent << "%" << ptr.getIndex()
                          << " (alloca): " << stateStr << std::endl;
            } else {
                std::cout << indent << "%" << ptr.getIndex()
                          << " (instruction result): " << stateStr << std::endl;
            }
        } else {
            // It's a block argument (function parameter)
            std::cout << indent << "%" << ptr.getIndex()
                      << " (function parameter): " << stateStr << std::endl;
        }
    }

    void mergeStatesFromPredecessors(gil::BasicBlock *bb)
    {
        auto preds = getPredecessors(bb);
        if (preds.empty()) {
            std::cout << "No predecessors to merge from (entry block)"
                      << std::endl;
            return;
        }

        std::cout << "MERGING states from " << preds.size()
                  << " predecessor(s):" << std::endl;

        bool first = true;
        llvm::DenseSet<gil::Value> inconsistentValues;

        for (auto *pred : preds) {
            auto predIt = blockEndStates.find(pred);
            if (predIt == blockEndStates.end()) {
                std::cout << "    Warning: Predecessor "
                          << pred->getLabel().str()
                          << " not analyzed yet - skipping" << std::endl;
                continue;
            }

            auto const &predState = predIt->second;
            std::cout << "\n  From predecessor '" << pred->getLabel().str()
                      << "' (" << predState.size()
                      << " variables):" << std::endl;

            // Show what's in the predecessor state
            for (auto const &entry : predState) {
                printVariableState(entry.first, entry.second, "    ");
            }

            if (first) {
                currentState = predState;
                first = false;
            } else {
                for (auto const &entry : predState) {
                    gil::Value ptr = entry.first;
                    MemoryState predMemState = entry.second;

                    auto currentIt = currentState.find(ptr);
                    if (currentIt != currentState.end()) {
                        if (currentIt->second != predMemState) {
                            inconsistentValues.insert(ptr);
                            // std::cout << "    Inconsistent state found for a
                            // "
                            //              "memory location"
                            //           << std::endl;
                        }
                    } else {
                        currentState[ptr] = MemoryState::Uninitialized;
                        if (predMemState != MemoryState::Uninitialized) {
                            inconsistentValues.insert(ptr);
                        }
                    }
                }

                for (auto const &entry : currentState) {
                    gil::Value ptr = entry.first;
                    if (predState.find(ptr) == predState.end()) {
                        inconsistentValues.insert(ptr);
                    }
                }
            }
        }

        for (gil::Value inconsistentPtr : inconsistentValues) {
            // std::cout << "ERROR: Inconsistent initialization state detected -
            // "
            //              "some paths initialize a variable, others don't"
            //           << std::endl;
        }

        for (gil::Value ptr : inconsistentValues) {
            currentState[ptr] = MemoryState::Uninitialized;
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

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILPASSES_DETECTUNINITIALIZED_HPP

// load take 면 그 다음에 uninitialzed
// 좋아
// allias structFieldPtn은
// ptr offset => operator []
