#ifndef GLU_SEMA_SCOPETABLE_HPP
#define GLU_SEMA_SCOPETABLE_HPP

#include "llvm/ADT/StringMap.h"
#include <llvm/ADT/SmallVector.h>

#include "AST/Decls.hpp"
#include "AST/Stmts.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

class ImportManager;

template <typename T> struct WithVisibility {
    ast::Visibility visibility;
    T item;
    WithVisibility(ast::Visibility visibility, T item)
        : visibility(visibility), item(item)
    {
    }

    // default constructor for lookup failures
    WithVisibility() : visibility(ast::Visibility::Private), item(nullptr) { }

    operator T() { return item; }
};

struct ScopeItem {
    /// @brief The possible overloads of the item.
    /// This is used to resolve overloaded functions and variables.
    /// The overloads are stored in a vector, optimized for the case where
    /// there is only one overload.
    llvm::SmallVector<WithVisibility<ast::DeclBase *>, 1> decls;
};

/// @brief Represents a scope's semantic table for semantic analysis.
/// This class is used to keep track of the items declared in a scope.
/// It is used to resolve names and types in the scope.
/// It is a hash table that maps names to their corresponding items.
/// There is a global scope for each module. Their namespaces will
/// reference other modules' global scopes.
/// Each function has a scope for itself, and a scope for the compound
/// statements within it. Each compound statement has its own scope.
class ScopeTable {
    /// @brief The parent scope table.
    ScopeTable *_parent;
    /// @brief The node this scope belongs to.
    /// For the global scope, this is the ModuleDecl.
    /// For local scopes, this is a CompoundStmt.
    ast::ASTNode *_node;
    /// @brief The types declared in this scope.
    /// Only the global scope has types.
    llvm::StringMap<WithVisibility<types::Ty>> _types;
    /// @brief The variables and functions declared in this scope.
    /// The global scope has functions and variables, local scopes have
    /// variables only.
    llvm::StringMap<ScopeItem> _items;
    /// @brief The namespaces declared in this scope.
    /// Only the global scope of a module can have namespaces.
    llvm::StringMap<WithVisibility<ScopeTable *>> _namespaces;

    /// @brief Synthetic functions generated during compilation (e.g.,
    /// @implement wrappers). Only the global scope has synthetic functions.
    llvm::SmallVector<ast::FunctionDecl *, 4> _syntheticFunctions;

public:
    /// @brief A special scope table representing the standard library
    /// namespace. This is used to resolve names in the standard library
    /// namespace. It is a global variable, initialized to nullptr.
    struct NamespaceBuiltinsOverloadToken { };
    /// @brief Creates the standard library namespace scope table.
    ScopeTable(NamespaceBuiltinsOverloadToken, ast::ASTContext *context);

    static ScopeTable BUILTINS_NS;

    /// @brief Creates a new local scope table using a node (e.g. a compound
    /// statement or function).
    /// @param parent The parent scope table.
    /// @param node The node this scope belongs to.
    ScopeTable(ScopeTable *parent, ast::ASTNode *node);

    /// @brief Generate a global scope table for a module
    /// @param node The module to visit
    /// @param importManager The import manager to use for resolving imports.
    ScopeTable(
        ast::ModuleDecl *node, ImportManager *importManager = nullptr,
        bool skipPrivateImports = false
    );

    /// @brief Inserts template parameter declarations into this scope.
    /// @param params The template parameter list to register.
    void insertTemplateParams(ast::TemplateParameterList *params);

    /// @brief Returns the parent scope table.
    ScopeTable *getParent() const { return _parent; }

    /// @brief Returns the node this scope belongs to.
    ast::ASTNode *getNode() const { return _node; }

    /// @brief Returns true if this scope is the global scope.
    /// The global scope is the root scope of the AST.
    /// It is the scope that contains all the types and functions declared
    /// in the module.
    bool isGlobalScope() const { return _parent == nullptr; }

    /// @brief Returns true if this scope is a function params scope.
    /// A function params scope is a scope that contains a function params
    /// declarations.
    bool isFunctionScope() const
    {
        return llvm::isa_and_nonnull<ast::FunctionDecl>(_node);
    }

    /// @brief Returns the root scope table (the global scope).
    /// This is used to resolve types in the global scope.
    ScopeTable *getGlobalScope()
    {
        if (_parent)
            return _parent->getGlobalScope();
        return this;
    }

    /// @brief Returns the module this scope belongs to.
    ast::ModuleDecl *getModule()
    {
        return llvm::cast<ast::ModuleDecl>(getGlobalScope()->getNode());
    }

    /// @brief Add a synthetic function to the global scope.
    /// Synthetic functions are compiler-generated functions like @implement
    /// wrappers.
    void addSyntheticFunction(ast::FunctionDecl *func)
    {
        assert(
            getGlobalScope() == this
            && "Only global scopes can have synthetic functions"
        );
        _syntheticFunctions.push_back(func);
    }

    /// @brief Get the synthetic functions for this module scope.
    llvm::ArrayRef<ast::FunctionDecl *> getSyntheticFunctions()
    {
        assert(
            getGlobalScope() == this
            && "Only global scopes have synthetic functions"
        );
        return _syntheticFunctions;
    }

    /// @brief Returns the function declaration this scope belongs to,
    /// or nullptr if this scope is the global scope.
    ast::FunctionDecl *getFunctionDecl();

    /// @brief Looks up a namespace defined directly in this scope.
    /// @param name The name of the namespace to look up.
    /// @return The ScopeTable if defined in this scope, nullptr otherwise.
    ScopeTable *getLocalNamespace(llvm::StringRef name)
    {
        return _namespaces.lookup(name);
    }

    /// @brief Looks up an item in the current scope or parent scopes.
    /// @param name The name of the item to look up.
    /// @return A pointer to the ScopeItem if found, or nullptr if not found.
    /// This is used to resolve overloaded functions and variables.
    /// Note that if there are multiple overloads, in different scopes,
    /// the ones in the closest scope are returned.
    ScopeItem *lookupItem(llvm::StringRef name);

    /// @brief Looks up a type in the current scope or parent scopes.
    /// @param name The name of the type to look up.
    /// @return A pointer to the DeclBase if found, or nullptr if not found.
    types::Ty lookupType(llvm::StringRef name);

    /// @brief Looks up a namespace in the current scope or parent scopes.
    /// @param name The name of the namespace to look up.
    /// @return The ScopeTable of the namespace if found, or nullptr if not
    /// found.
    ScopeTable *lookupNamespace(llvm::StringRef name);

    /// @brief Looks up an item in the given namespace, or the current scope
    /// for the empty namespace.
    /// @param ident the namespaced identifier
    /// @return A pointer to the ScopeItem if found, or nullptr if not found.
    /// This is used to resolve overloaded functions and variables.
    /// Note that if there are multiple overloads, in different scopes,
    /// the ones in the closest scope are returned.
    ScopeItem *lookupItem(ast::NamespaceIdentifier ident);

    /// @brief Looks up a type in the given namespace, or the current scope.
    /// @param ident the namespaced identifier
    /// @return A pointer to the DeclBase if found, or nullptr if not found.
    types::Ty lookupType(ast::NamespaceIdentifier ident);

    /// @brief Inserts a new item in the current scope.
    /// @param name The name of the item to insert.
    /// @param item The item to insert.
    void insertItem(
        llvm::StringRef name, ast::DeclBase *item, ast::Visibility visibility
    );

    /// @brief Inserts a new type in the current scope.
    /// @param name The name of the type to insert.
    /// @param type The type to insert.
    /// @return True if the type was inserted, false if it already exists.
    bool
    insertType(llvm::StringRef name, types::Ty type, ast::Visibility visibility)
    {
        return _types.insert({ name, { visibility, type } }).second;
    }

    /// @brief Inserts a new namespace in the current scope.
    /// @param name The name of the namespace to insert.
    /// @param table The scope table of the namespace.
    /// @return True if the namespace was inserted, false if it already exists.
    bool insertNamespace(
        llvm::StringRef name, ScopeTable *table, ast::Visibility visibility
    )
    {
        return _namespaces.insert({ name, { visibility, table } }).second;
    }

    /// @brief Copies all items from this scope to another scope.
    /// @param other The target scope to copy items into.
    /// @param selector The selector to use for the copy (a function returning
    /// true if the item should be copied).
    /// @param diag The diagnostic manager to report errors.
    /// @param loc The location of the copy (import) operation.
    /// @param importVisibility The visibility to use for imported items. Public
    /// will re-export the items, private will not.
    /// @return True if the copy was successful, false if there were errors.
    bool copyInto(
        ScopeTable *other,
        std::function<llvm::StringRef(llvm::StringRef)> selector,
        DiagnosticManager &diag, SourceLocation loc,
        ast::Visibility importVisibility
    );
};

} // namespace glu::sema

#endif // GLU_SEMA_SCOPETABLE_HPP
