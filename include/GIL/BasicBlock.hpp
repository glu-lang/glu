#ifndef GLU_GIL_BASICBLOCK_HPP
#define GLU_GIL_BASICBLOCK_HPP

#include <llvm/ADT/ilist.h>
#include <llvm/ADT/ilist_node.h>
#include <string>

// ! TODO: move the node impl of InstBase in InstBase file
namespace glu {
namespace gil {
class InstBase;
class BasicBlock;
} // end namespace gil
} // end namespace glu

namespace llvm {
namespace ilist_detail {

class InstListBase : public ilist_base<false> {
public:
    template <class T> static void remove(T &N)
    {
        removeImpl(N);
    }

    template <class T> static void insertBefore(T &Next, T &N)
    {
        insertBeforeImpl(Next, N);
    }

    template <class T> static void transferBefore(T &Next, T &First, T &Last)
    {
        transferBeforeImpl(Next, First, Last);
    }
};

template <> struct compute_node_options<glu::gil::InstBase> {
    struct type {
        using value_type = glu::gil::InstBase;
        using pointer = value_type *;
        using reference = value_type &;
        using const_pointer = value_type const *;
        using const_reference = value_type const &;

        static bool const enable_sentinel_tracking = false;
        static bool const is_sentinel_tracking_explicit = false;
        static bool const has_iterator_bits = false;
        using tag = void;
        using node_base_type = ilist_node_base<enable_sentinel_tracking>;
        using list_base_type = InstListBase;
    };
};

} // end namespace ilist_detail
} // end llvm namespace

namespace glu {
namespace gil {

class BasicBlock;
class InstBase : public llvm::ilist_node<InstBase> {
public:
    friend llvm::ilist_traits<InstBase>;
    friend BasicBlock;

    BasicBlock *parent = nullptr;

    void setParent(BasicBlock *p)
    {
        parent = p;
    }

    inline BasicBlock *getParent() const
    {
        return parent;
    }

    inline BasicBlock *getParent()
    {
        return parent;
    }

    InstBase() {};
    virtual ~InstBase() = default;
    InstBase(InstBase const &) = default;
};

class TerminatorInst : public InstBase { };

} // end namespace gil
} // end namespace glu

//===----------------------------------------------------------------------===//
// ilist_traits for InstBase
//===----------------------------------------------------------------------===//

namespace llvm {

template <>
struct ilist_traits<glu::gil::InstBase>
    : public ilist_node_traits<glu::gil::InstBase> {
private:
    glu::gil::BasicBlock *getContainingBlock();

public:
    void addNodeToList(glu::gil::InstBase *I)
    {
        I->parent = getContainingBlock();
    }

private:
    void createNode(glu::gil::InstBase const &);
};

} // end namespace llvm

//===----------------------------------------------------------------------===//

namespace glu {
namespace gil {

/**
 * @class BasicBlock
 * @brief Represents a basic block for instructions in the GIL (Glu Intermediate
 * Language).
 * See the documentation here for more information:
 * https://glu-lang.org/gil/#basic-blocks
 */
class BasicBlock {

public:
    using InstListType = llvm::iplist<InstBase>;

private:
    InstListType _instructions;
    std::string _label;

public:
    BasicBlock(std::string label = "")
        : _label(label) {};
    ~BasicBlock() = default;

    inline InstListType const &getInstructions() const
    {
        return _instructions;
    }

    inline std::size_t getInstructionCount() const
    {
        return _instructions.size();
    }

    InstBase *popFirstInstruction();

    inline void addInstructionAtEnd(InstBase *inst)
    {
        _instructions.push_back(inst);
    }

    inline void addInstructionAtStart(InstBase *inst)
    {
        _instructions.push_front(inst);
    }

    inline void addInstructionAt(InstBase *inst, InstListType::iterator it)
    {
        _instructions.insert(it, inst);
    }

    void addInstructionBefore(InstBase *inst, InstBase *before);
    void addInstructionAfter(InstBase *inst, InstBase *after);
    void replaceInstruction(InstBase *oldInst, InstBase *newInst);
    void removeInstruction(InstBase *inst);

    // defined to be used by ilist
    inline static InstListType BasicBlock::*getSublistAccess(InstBase *)
    {
        return &BasicBlock::_instructions;
    }

    InstBase const *getTerminator() const;
    InstBase *getTerminator();
    TerminatorInst *getTerminatorInst() const;
    void setTerminator(InstBase *terminator);

    inline void setLabel(std::string label)
    {
        _label = label;
    }

    inline std::string const &getLabel() const
    {
        return _label;
    }
};

} // end namespace gil
} // end namespace glu

#endif // GLU_GIL_BASICBLOCK_HPP
