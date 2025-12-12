#ifndef GLU_GIL_GLOBAL_HPP
#define GLU_GIL_GLOBAL_HPP

#include "AST/Decls.hpp"
#include "Types.hpp"

#include <llvm/ADT/ilist_node.h>

namespace glu::gil {
// Forward declarations
class Module;
class Function;
class Global;
}

namespace glu::gil {

/// @class Global
/// @brief Represents a global constant/variable in GIL (Glu Intermediate
/// Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil
class Global : public llvm::ilist_node<Global, llvm::ilist_parent<Module>> {
    using NodeBase = llvm::ilist_node<Global, llvm::ilist_parent<Module>>;

private:
    friend llvm::ilist_traits<Global>;
    friend class Module; // Allow Module to set itself as the parent
                         // when added

    llvm::StringRef _name;
    glu::types::TypeBase *_type;
    Function *_initializer;
    Function *_destructor = nullptr;
    glu::ast::VarLetDecl *_decl;

    /// @brief Indicates whether the global has an initializer function.
    bool _hasInitializer = false;

public:
    Global(
        llvm::StringRef name, glu::types::TypeBase *type, bool hasInitializer,
        glu::ast::VarLetDecl *decl
    )
        : _name(name)
        , _type(type)
        , _initializer(nullptr)
        , _decl(decl)
        , _hasInitializer(hasInitializer)
    {
    }

    llvm::StringRef const &getName() const { return _name; }
    void setName(llvm::StringRef const &name) { _name = name; }

    glu::types::TypeBase *getType() const { return _type; }

    Function *getInitializer() const { return _initializer; }
    void setInitializer(Function *initializer)
    {
        assert(_hasInitializer && "Global does not have an initializer");
        _initializer = initializer;
    }
    bool hasInitializer() const { return _hasInitializer; }

    Function *getDestructor() const { return _destructor; }
    void setDestructor(Function *destructor) { _destructor = destructor; }

    /// Returns the parent module of this function
    Module *getParent() const
    {
        return const_cast<Global *>(this)->NodeBase::getParent();
    }
    /// Set the parent module of this function
    void setParent(Module *parent) { this->NodeBase::setParent(parent); }

    glu::ast::VarLetDecl *getDecl() const { return _decl; }
    void setDecl(glu::ast::VarLetDecl *decl) { _decl = decl; }

    /// @brief Print a human-readable representation of this function to
    /// standard output, for debugging purposes.
    void print();
};

} // end namespace glu::gil

///===----------------------------------------------------------------------===//
/// ilist_traits for Global
///===----------------------------------------------------------------------===//
namespace llvm {

template <>
struct ilist_traits<glu::gil::Global>
    : public ilist_node_traits<glu::gil::Global> {
private:
    glu::gil::Module *getContainingModule();

public:
    void addNodeToList(glu::gil::Global *global)
    {
        global->setParent(getContainingModule());
    }

    void removeNodeFromList(glu::gil::Global *global)
    {
        global->setParent(nullptr);
    }
};

} // end namespace llvm

#endif // GLU_GIL_GLOBAL_HPP
