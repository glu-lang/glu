#include "ScopeTable.hpp"
#include "AST/Types/EnumTy.hpp"
#include "AST/Types/StructTy.hpp"
#include "AST/Types/TypeAliasTy.hpp"
#include "Expr/RefExpr.hpp"

namespace glu::sema {

ScopeTable::ScopeTable(ScopeTable *parent, ast::ASTNode *node)
    : _parent(parent), _node(node)
{
    assert(parent && "Parent scope must be provided");
    assert(node && "Node must be provided for local scopes");
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
        return it->second;
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

void ScopeTable::insertItem(
    llvm::StringRef name, ast::DeclBase *item, ast::Visibility visibility
)
{
    assert(
        (llvm::isa<ast::VarLetDecl>(item) || llvm::isa<ast::FunctionDecl>(item))
        && "Item must be a variable or function declaration"
    );
    auto it = _items.find(name);
    if (it != _items.end()) {
        it->second.decls.push_back({ visibility, item });
    } else {
        ScopeItem scopeItem { { { visibility, item } } };
        _items.insert({ name, scopeItem });
    }
}

bool ScopeTable::copyInto(
    ScopeTable *other, std::function<bool(llvm::StringRef)> selector,
    DiagnosticManager &diag, SourceLocation loc,
    ast::Visibility importVisibility
)
{
    bool found = false;
    for (auto &item : _items) {
        if (!selector(item.first()))
            continue;

        // Only copy public items during imports
        bool hasPublicDecl = false;
        for (auto &decl : item.second.decls) {
            if (decl.visibility == ast::Visibility::Public) {
                hasPublicDecl = true;
                break;
            }
        }
        if (!hasPublicDecl)
            continue;

        found = true;
        ScopeItem publicItem;
        if (auto *existing = other->lookupItem(item.first())) {
            publicItem = *existing;
        }
        // Only copy the public declarations
        for (auto decl : item.second.decls) {
            if (decl.visibility == ast::Visibility::Public) {
                publicItem.decls.push_back({ importVisibility, decl });
            }
        }
        other->_items[item.first()] = publicItem;
    }

    for (auto &type : _types) {
        if (!selector(type.first()))
            continue;

        // Builtins are always private (never exported)
        bool isPublic = type.second.visibility == ast::Visibility::Public;

        if (!isPublic)
            continue;

        found = true;
        if (other->lookupType(type.first())) {
            // Type already exists in the target scope, report conflict
            diag.error(
                loc,
                "Type '" + type.first().str()
                    + "' already exists in scope and conflicts with imported "
                      "type."
            );
            continue;
        }
        other->insertType(type.first(), type.second, importVisibility);
    }
    for (auto &ns : _namespaces) {
        if (!selector(ns.first()))
            continue;

        // Builtin namespaces are private and should not be exported
        if (ns.second.visibility == ast::Visibility::Private)
            continue;

        found = true;
        if (other->lookupNamespace(ns.first())) {
            // Namespace already exists in the target scope, report conflict
            diag.error(
                loc,
                "Namespace '" + ns.first().str()
                    + "' already exists in scope and conflicts with imported "
                      "namespace."
            );
            continue;
        }
        other->_namespaces.insert(
            { ns.first(), { importVisibility, ns.second } }
        );
    }
    return found;
}

} // namespace glu::sema
