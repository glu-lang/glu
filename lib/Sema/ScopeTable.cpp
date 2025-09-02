#include "ScopeTable.hpp"
#include "Expr/RefExpr.hpp"

namespace glu::sema {

ScopeTable::ScopeTable(ScopeTable *parent, ast::FunctionDecl *node)
    : _parent(parent), _node(node)
{
    assert(parent && "Parent scope must be provided");
    assert(node && "Node must be provided for local scopes (FunctionDecl)");
}

ScopeTable::ScopeTable(ScopeTable *parent, ast::StmtBase *node)
    : _parent(parent), _node(node)
{
    assert(parent && "Parent scope must be provided");
    assert(node && "Node must be provided for local scopes (CompoundStmt)");
}

ast::FunctionDecl *ScopeTable::getFunctionDecl()
{
    if (isGlobalScope())
        return nullptr;
    if (isFunctionScope())
        return llvm::cast<ast::FunctionDecl>(_node);
    return _parent->getFunctionDecl();
}

ScopeItem *ScopeTable::lookupItem(llvm::StringRef name)
{
    auto it = _items.find(name);
    if (it != _items.end())
        return &it->second;
    if (_parent)
        return _parent->lookupItem(name);
    return nullptr;
}

types::Ty ScopeTable::lookupType(llvm::StringRef name)
{
    // Note: only the global scope should have types, but we check all
    // scopes because why not
    auto it = _types.find(name);
    if (it != _types.end())
        return it->second;
    if (_parent)
        return _parent->lookupType(name);
    return nullptr;
}

ScopeTable *ScopeTable::lookupNamespace(llvm::StringRef name)
{
    auto it = _namespaces.find(name);
    if (it != _namespaces.end())
        return &it->second;
    if (_parent)
        return _parent->lookupNamespace(name);
    return nullptr;
}

ScopeItem *ScopeTable::lookupItem(ast::NamespaceIdentifier ident)
{
    if (ident.components.empty())
        return lookupItem(ident.identifier);
    auto scope = lookupNamespace(ident.components[0]);
    if (!scope)
        return nullptr;

    return scope->lookupItem(
        ast::NamespaceIdentifier { ident.components.drop_front(),
                                   ident.identifier }
    );
}

types::Ty ScopeTable::lookupType(ast::NamespaceIdentifier ident)
{
    if (ident.components.empty())
        return lookupType(ident.identifier);
    auto scope = lookupNamespace(ident.components[0]);
    if (!scope)
        return nullptr;

    return scope->lookupType(
        ast::NamespaceIdentifier { ident.components.drop_front(),
                                   ident.identifier }
    );
}

void ScopeTable::insertItem(llvm::StringRef name, ast::DeclBase *item)
{
    assert(
        llvm::isa<ast::VarLetDecl>(item)
        || llvm::isa<ast::FunctionDecl>(item)
            && "Item must be a variable or function declaration"
    );
    auto it = _items.find(name);
    if (it != _items.end()) {
        it->second.decls.push_back(item);
    } else {
        ScopeItem scopeItem { { item } };
        _items.insert({ name, scopeItem });
    }
}

}
