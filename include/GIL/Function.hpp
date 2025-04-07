#ifndef GLU_GIL_FUNCTION_HPP
#define GLU_GIL_FUNCTION_HPP

#include "BasicBlock.hpp"
#include "Types.hpp"

namespace llvm::ilist_detail {
class FunctionListBase : public ilist_base<false> {
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

template <> struct compute_node_options<glu::gil::Function> {
    struct type {
        using value_type = glu::gil::Function;
        using pointer = value_type *;
        using reference = value_type &;
        using const_pointer = value_type const *;
        using const_reference = value_type const &;

        static bool const enable_sentinel_tracking = false;
        static bool const is_sentinel_tracking_explicit = false;
        static bool const has_iterator_bits = false;
        using tag = void;
        using node_base_type = ilist_node_base<enable_sentinel_tracking>;
        using list_base_type = FunctionListBase;
    };
};

} // end namespace llvm::ilist_detail

namespace glu::gil {

// Forward declarations
class Module;

/// @class Function
/// @brief Represents a function in the GIL (Glu Intermediate Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil
class Function : public llvm::ilist_node<Function> {

public:
    using BBListType = llvm::iplist<BasicBlock>;

private:
    Module *_parentModule = nullptr;
    friend llvm::ilist_traits<Function>;
    friend class Module; // Allow Module to set itself as the parent
                         // when added

    BBListType _basicBlocks;
    llvm::StringRef _name;
    glu::types::FunctionTy *_type;

public:
    Function(llvm::StringRef name, glu::types::FunctionTy *type)
        : _name(name), _type(type)
    {
    }

    ~Function() = default;

    // defined to be used by ilist
    static BBListType Function::*getSublistAccess(BasicBlock *)
    {
        return &Function::_basicBlocks;
    }

    llvm::StringRef const &getName() const { return _name; }
    void setName(llvm::StringRef const &name) { _name = name; }

    glu::types::FunctionTy *getType() const { return _type; }

    BBListType &getBasicBlocks() { return _basicBlocks; }
    BasicBlock *getEntryBlock() { return &_basicBlocks.front(); }
    std::size_t getBasicBlockCount() const { return _basicBlocks.size(); }

    void addBasicBlockBefore(BasicBlock *bb, BasicBlock *before);
    void addBasicBlockAfter(BasicBlock *bb, BasicBlock *before);
    void addBasicBlockAtEnd(BasicBlock *bb) { _basicBlocks.push_back(bb); }
    void addBasicBlockAtStart(BasicBlock *bb) { _basicBlocks.push_front(bb); }
    void addBasicBlockAt(BasicBlock *bb, BBListType::iterator it)
    {
        _basicBlocks.insert(it, bb);
    }

    void replaceBasicBlock(BasicBlock *oldBB, BasicBlock *newBB);

    void removeBasicBlock(BBListType::iterator it) { _basicBlocks.erase(it); }
    void removeBasicBlock(BasicBlock *bb) { _basicBlocks.remove(bb); }

    /// Returns the parent module of this function
    Module *getParent() const { return _parentModule; }
    /// Set the parent module of this function
    void setParent(Module *parent) { _parentModule = parent; }
};

} // end namespace glu::gil

///===----------------------------------------------------------------------===//
/// ilist_traits for Function
///===----------------------------------------------------------------------===//
namespace llvm {

template <>
struct ilist_traits<glu::gil::Function>
    : public ilist_node_traits<glu::gil::Function> {
private:
    glu::gil::Module *getContainingModule();

public:
    void addNodeToList(glu::gil::Function *function)
    {
        function->_parentModule = getContainingModule();
    }

    /// @brief This is called by the ilist and should not be called directly.
    /// This is used to delete the pointer to the function when it is removed
    /// from the list. The pointer to the object function should have always
    /// been created with new because it should only be created in the context
    /// of a module.
    /// @param function A pointer to the function to delete
    void deleteNode(glu::gil::Function *function)
    {
        if (function != nullptr)
            delete function;
    }

private:
    void createNode(glu::gil::Function const &);
};

} // end namespace llvm

#endif // GLU_GIL_FUNCTION_HPP
