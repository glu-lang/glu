#ifndef GLU_SEMA_SCOPETABLE_HPP
#define GLU_SEMA_SCOPETABLE_HPP

#include "llvm/ADT/StringMap.h"

#include "AST/Decls.hpp"
#include "AST/Stmts.hpp"

namespace glu::sema {

struct ScopeItem {
    /// @brief The possible overloads of the item.
    /// This is used to resolve overloaded functions and variables.
    /// The overloads are stored in a vector, optimized for the case where
    /// there is only one overload.
    llvm::SmallVector<ast::DeclBase *, 1> decls;
};

/// @brief Represents a scope's semantic table for semantic analysis.
/// This class is used to keep track of the items declared in a scope.
/// It is used to resolve names and types in the scope.
/// It is a hash table that maps names to their corresponding items.
class ScopeTable {
    /// @brief The parent scope table.
    ScopeTable *_parent;
    /// @brief The node this scope belongs to.
    /// For the global scope, this is the ModuleDecl.
    /// For local scopes, this is a CompoundStmt.
    ast::ASTNode *_node;
    /// @brief The types declared in this scope.
    /// Only the global scope has types.
    llvm::StringMap<types::Ty> _types;
    /// @brief The variables and functions declared in this scope.
    /// The global scope has functions and variables, local scopes have
    /// variables only.
    llvm::StringMap<ScopeItem> _items;

public:
    /// @brief Creates a new local scope table for a for binding decl.
    /// @param parent The parent scope table.
    /// @param node The node this scope belongs to.
    ScopeTable(ScopeTable *parent, ast::ForStmt *node);

    /// @brief Creates a new local scope table for a Function params.
    /// @param parent The parent scope table.
    /// @param node The node this scope belongs to.
    ScopeTable(ScopeTable *parent, ast::FunctionDecl *node);

    /// @brief Creates a new local scope table using a compoundStmt.
    /// @param parent The parent scope table.
    /// @param node The node this scope belongs to.
    ScopeTable(ScopeTable *parent, ast::CompoundStmt *node);

    /// @brief Generate a global scope table for a module
    /// @param node The module to visit
    ScopeTable(ast::ModuleDecl *node);

    /// @brief Returns the parent scope table.
    ScopeTable *getParent() const { return _parent; }

    /// @brief Returns the node this scope belongs to.
    ast::ASTNode *getNode() const { return _node; }

    /// @brief Returns true if this scope is the global scope.
    /// The global scope is the root scope of the AST.
    /// It is the scope that contains all the types and functions declared
    /// in the module.
    bool isGlobalScope() const { return _parent == nullptr; }

    /// @brief Returns true if this scope is a for scope.
    /// A for scope is a scope that contains a for binding declaration.
    bool isForScope() const
    {
        return llvm::isa_and_nonnull<ast::ForStmt>(_node);
    }

    /// @brief Returns true if this scope is a function params scope.
    /// A function params scope is a scope that contains a function params
    /// declarations.
    bool isFunctionScope() const
    {
        return llvm::isa_and_nonnull<ast::FunctionDecl>(_node);
    }

    /// @brief Returns true if this scope is an unnamed scope.
    /// An unnamed scope is a simple {} block within a function.
    bool isUnnamedScope() const
    {
        return !isGlobalScope() && !isFunctionScope();
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

    /// @brief Returns the function declaration this scope belongs to,
    /// or nullptr if this scope is the global scope.
    ast::FunctionDecl *getFunctionDecl();

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

    /// @brief Inserts a new item in the current scope.
    /// @param name The name of the item to insert.
    /// @param item The item to insert.
    void insertItem(llvm::StringRef name, ast::DeclBase *item);

    /// @brief Inserts a new type in the current scope.
    /// @param name The name of the type to insert.
    /// @param type The type to insert.
    /// @return True if the type was inserted, false if it already exists.
    bool insertType(llvm::StringRef name, types::Ty type)
    {
        return _types.insert({ name, type }).second;
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SCOPETABLE_HPP
