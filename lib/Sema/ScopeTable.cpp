#include "ScopeTable.hpp"
#include "Expr/RefExpr.hpp"
#include "AST/Types/StructTy.hpp"
#include "AST/Types/EnumTy.hpp"
#include "AST/Types/TypeAliasTy.hpp"

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

void ScopeTable::insertItem(llvm::StringRef name, ast::DeclBase *item)
{
    assert(
        (llvm::isa<ast::VarLetDecl>(item)
        || llvm::isa<ast::FunctionDecl>(item)
        || llvm::isa<ast::TypeDecl>(item))
            && "Item must be a variable, function, or type declaration"
    );
    auto it = _items.find(name);
    if (it != _items.end()) {
        it->second.decls.push_back(item);
    } else {
        ScopeItem scopeItem { { item } };
        _items.insert({ name, scopeItem });
    }
}

bool ScopeTable::copyInto(
    ScopeTable *other, llvm::StringRef selector, DiagnosticManager &diag,
    SourceLocation loc
)
{
    bool found = false;
    for (auto &item : _items) {
        if (selector != "*" && item.first() != selector)
            continue;
        
        // Only copy public items during imports
        bool hasPublicDecl = false;
        for (auto *decl : item.second.decls) {
            if (decl->isPublic()) {
                hasPublicDecl = true;
                break;
            }
        }
        if (!hasPublicDecl)
            continue;
            
        found = true;
        if (other->lookupItem(item.first())) {
            // Item already exists in the target scope.
            diag.error(
                loc,
                "Item '" + item.first().str()
                    + "' already exists in scope and conflicts with imported "
                      "item."
            );
            continue;
        }
        
        // Only copy the public declarations
        ScopeItem publicItem;
        for (auto *decl : item.second.decls) {
            if (decl->isPublic()) {
                publicItem.decls.push_back(decl);
            }
        }
        other->_items.insert({ item.first(), publicItem });
    }
    for (auto &type : _types) {
        if (selector != "*" && type.first() != selector)
            continue;
            
        // Check if the type declaration is public
        bool isPublic = false;
        if (auto *structTy = llvm::dyn_cast<glu::types::StructTy>(type.second)) {
            isPublic = structTy->getDecl()->isPublic();
        } else if (auto *enumTy = llvm::dyn_cast<glu::types::EnumTy>(type.second)) {
            isPublic = enumTy->getDecl()->isPublic();
        } else {
            // For other types (like TypeAliasTy), check if there's a corresponding
            // type declaration in the items with the same name
            auto itemIt = _items.find(type.first());
            if (itemIt != _items.end()) {
                for (auto *decl : itemIt->second.decls) {
                    if (llvm::isa<ast::TypeDecl>(decl)) {
                        isPublic = decl->isPublic();
                        break;
                    }
                }
            } else {
                // Built-in types are generally public
                isPublic = true;
            }
        }
        
        if (!isPublic)
            continue;
            
        found = true;
        if (other->lookupType(type.first())) {
            // Type already exists in the target scope.
            diag.error(
                loc,
                "Type '" + type.first().str()
                    + "' already exists in scope and conflicts with imported "
                      "type."
            );
            continue;
        }
        other->_types.insert({ type.first(), type.second });
    }
    for (auto &ns : _namespaces) {
        if (selector != "*" && ns.first() != selector)
            continue;
        found = true;
        if (other->lookupNamespace(ns.first())) {
            // Namespace already exists in the target scope.
            diag.error(
                loc,
                "Namespace '" + ns.first().str()
                    + "' already exists in scope and conflicts with imported "
                      "namespace."
            );
            continue;
        }
        other->_namespaces.insert({ ns.first(), ns.second });
    }
    return found;
}

} // namespace glu::sema
