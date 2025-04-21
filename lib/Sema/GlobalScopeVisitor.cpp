#include "ScopeTable.hpp"

#include "AST/ASTVisitor.hpp"

namespace glu::sema {

class GlobalScopeVisitor
    : public glu::ast::ASTVisitor<GlobalScopeVisitor, void> {
    /// @brief The scope table we are populating.
    glu::sema::ScopeTable *_scopeTable;

public:
    /// @brief Constructor.
    /// @param scopeTable The scope table to populate.
    explicit GlobalScopeVisitor(ScopeTable *scopeTable)
        : _scopeTable(scopeTable)
    {
    }

    void visitModuleDecl(ast::ModuleDecl *node)
    {
        for (auto *decl : node->getDecls()) {
            visit(decl);
        }
    }

    void visitTypeDecl(ast::TypeDecl *node)
    {
        _scopeTable->insertType(node->getName(), node);
    }

    void visitFunctionDecl(ast::FunctionDecl *node)
    {
        _scopeTable->insertItem(node->getName(), node);
    }

    void visitVarLetDecl(ast::VarLetDecl *node)
    {
        _scopeTable->insertItem(node->getName(), node);
    }
};

ScopeTable::ScopeTable(ast::ModuleDecl *node) : _parent(nullptr), _node(node)
{
    assert(node && "Node must be provided for global scope (ModuleDecl)");
    GlobalScopeVisitor(this).visit(node);
}

} // namespace glu::sema
