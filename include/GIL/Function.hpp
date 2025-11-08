#ifndef GLU_GIL_FUNCTION_HPP
#define GLU_GIL_FUNCTION_HPP

#include "AST/Decls.hpp"
#include "BasicBlock.hpp"
#include "Types.hpp"

namespace glu::gil {

// Forward declarations
class Module;

/// @class Function
/// @brief Represents a function in the GIL (Glu Intermediate Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil
class Function : public llvm::ilist_node<Function, llvm::ilist_parent<Module>> {
    using NodeBase = llvm::ilist_node<Function, llvm::ilist_parent<Module>>;

public:
    using BBListType = llvm::iplist<BasicBlock, llvm::ilist_parent<Function>>;

private:
    friend llvm::ilist_traits<Function>;
    friend class Module; // Allow Module to set itself as the parent
                         // when added

    BBListType _basicBlocks;
    llvm::StringRef _name;
    glu::types::FunctionTy *_type;
    glu::ast::FunctionDecl *_decl;

public:
    Function(
        llvm::StringRef name, glu::types::FunctionTy *type,
        glu::ast::FunctionDecl *decl
    )
        : _name(name), _type(type), _decl(decl)
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
    void setType(glu::types::FunctionTy *type) { _type = type; }

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
    Module *getParent() const
    {
        return const_cast<Function *>(this)->NodeBase::getParent();
    }
    /// Set the parent module of this function
    void setParent(Module *parent) { this->NodeBase::setParent(parent); }

    glu::ast::FunctionDecl *getDecl() const { return _decl; }

    void setDecl(glu::ast::FunctionDecl *decl) { _decl = decl; }

    /// @brief Print a human-readable representation of this function to
    /// standard output, for debugging purposes.
    void print();
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
        function->setParent(getContainingModule());
    }

    void removeNodeFromList(glu::gil::Function *function)
    {
        function->setParent(nullptr);
    }
};

} // end namespace llvm

#endif // GLU_GIL_FUNCTION_HPP
