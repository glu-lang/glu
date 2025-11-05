#ifndef GLU_GIL_GLOBAL_HPP
#define GLU_GIL_GLOBAL_HPP

#include "AST/Decls.hpp"
#include "Types.hpp"

namespace glu::gil {
// Forward declarations
class Module;
class Function;
class Global;
}

namespace llvm::ilist_detail {
class GlobalListBase : public ilist_base<false, void> {
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

template <> struct compute_node_options<glu::gil::Global> {
    struct type {
        using value_type = glu::gil::Global;
        using pointer = value_type *;
        using reference = value_type &;
        using const_pointer = value_type const *;
        using const_reference = value_type const &;

        static bool const enable_sentinel_tracking = false;
        static bool const is_sentinel_tracking_explicit = false;
        static bool const has_iterator_bits = false;
        using tag = void;
        using parent_ty = void;
        using node_base_type =
            ilist_node_base<enable_sentinel_tracking, parent_ty>;
        using list_base_type = GlobalListBase;
    };
};

} // end namespace llvm::ilist_detail

namespace glu::gil {

/// @class Global
/// @brief Represents a global constant/variable in GIL (Glu Intermediate
/// Language).
///
/// See the documentation here for more information:
/// https://glu-lang.org/gil
class Global : public llvm::ilist_node<Global> {
private:
    Module *_parentModule = nullptr;
    friend llvm::ilist_traits<Global>;
    friend class Module; // Allow Module to set itself as the parent
                         // when added

    llvm::StringRef _name;
    glu::types::TypeBase *_type;
    Function *_initializer;
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

    /// Returns the parent module of this function
    Module *getParent() const { return _parentModule; }
    /// Set the parent module of this function
    void setParent(Module *parent) { _parentModule = parent; }

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
        global->_parentModule = getContainingModule();
    }

    /// @brief This is called by the ilist and should not be called directly.
    /// This is used to delete the pointer to the global when it is removed
    /// from the list. The pointer to the object global should have always
    /// been created with new because it should only be created in the context
    /// of a module.
    /// @param global A pointer to the global to delete
    void deleteNode(glu::gil::Global *)
    {
        // We do not delete the global here because it may be owned by
        // BumpPtrAllocator
    }

private:
    void createNode(glu::gil::Global const &);
};

} // end namespace llvm

#endif // GLU_GIL_GLOBAL_HPP
