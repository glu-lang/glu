#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include "InstBase.hpp"

#include <string>

namespace llvm::ilist_detail {
class BasicBlockListBase : public ilist_base<false> {
public:
    template <class T> static void remove(T &N) { removeImpl(N); }

    template <class T> static void insertBefore(T &Next, T &N)
    {
        insertBeforeImpl(Next, N);
    }

    template <class T> static void transferBefore(T &Next, T &First, T &Last)
    {
        transferBeforeImpl(Next, First, Last);
    }
};

template <> struct compute_node_options<glu::gil::BasicBlock> {
    struct type {
        using value_type = glu::gil::BasicBlock;
        using pointer = value_type *;
        using reference = value_type &;
        using const_pointer = value_type const *;
        using const_reference = value_type const &;

        static bool const enable_sentinel_tracking = false;
        static bool const is_sentinel_tracking_explicit = false;
        static bool const has_iterator_bits = false;
        using tag = void;
        using node_base_type = ilist_node_base<enable_sentinel_tracking>;
        using list_base_type = BasicBlockListBase;
    };
};

} // end namespace llvm::ilist_detail

namespace glu::gil {

class Function;

// TODO: add parameters
/// @class BasicBlock
/// @brief Represents a basic block for instructions in the GIL (Glu
/// Intermediate Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil/#basic-blocks
class BasicBlock : public llvm::ilist_node<BasicBlock> {

public:
    using InstListType = llvm::iplist<InstBase>;

private:
    /// The parent function of this basic block
    Function *parent = nullptr;
    friend llvm::ilist_traits<BasicBlock>;
    friend class Function; // Allow Function to set itself as the parent
                           // when added

    InstListType _instructions;
    std::string _label;

public:
    BasicBlock(std::string label = "") : _label(label) { };
    ~BasicBlock() = default;

    InstListType const &getInstructions() const { return _instructions; }

    std::size_t getInstructionCount() const { return _instructions.size(); }

    InstBase *popFirstInstruction();

    void addInstructionAtEnd(InstBase *inst) { _instructions.push_back(inst); }

    void addInstructionAtStart(InstBase *inst)
    {
        _instructions.push_front(inst);
    }

    void addInstructionAt(InstBase *inst, InstListType::iterator it)
    {
        _instructions.insert(it, inst);
    }

    void addInstructionBefore(InstBase *inst, InstBase *before);
    void addInstructionAfter(InstBase *inst, InstBase *after);
    void replaceInstruction(InstBase *oldInst, InstBase *newInst);
    void removeInstruction(InstBase *inst);

    InstBase *getTerminator();
    TerminatorInst *getTerminatorInst();
    void setTerminator(InstBase *terminator);

    void setLabel(std::string label) { _label = label; }
    std::string const &getLabel() const { return _label; }

    /// defined to be used by ilist
    static InstListType BasicBlock::*getSublistAccess(InstBase *)
    {
        return &BasicBlock::_instructions;
    }

    /// Returns the parent function of this basic block
    Function *getParent() const { return parent; }

    /// Set the parent function of this basic block
    void setParent(Function *parent) { parent = parent; }
};

} // end namespace glu::gil

///===----------------------------------------------------------------------===//
/// ilist_traits for BasicBlock
///===----------------------------------------------------------------------===//
namespace llvm {

template <>
struct ilist_traits<glu::gil::BasicBlock>
    : public ilist_node_traits<glu::gil::BasicBlock> {
private:
    glu::gil::Function *getContainingFunction();

public:
    void addNodeToList(glu::gil::BasicBlock *block)
    {
        block->parent = getContainingFunction();
    }

private:
    void createNode(glu::gil::BasicBlock const &);
};

} // end namespace llvm

#endif // GLU_GIL_BASICBLOCK_HPP
