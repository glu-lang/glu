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
        auto &types
            = _scopeTable->getModule()->getContext()->getTypesMemoryArena();
        _scopeTable->insertType(
            "Int", types.create<types::IntTy>(types::IntTy::Signed, 32)
        );
        _scopeTable->insertType(
            "Float", types.create<types::FloatTy>(types::FloatTy::FLOAT)
        );
        _scopeTable->insertType(
            "Double", types.create<types::FloatTy>(types::FloatTy::DOUBLE)
        );
        _scopeTable->insertType("Bool", types.create<types::BoolTy>());
        _scopeTable->insertType("Char", types.create<types::CharTy>());
        _scopeTable->insertType("Void", types.create<types::VoidTy>());
        _scopeTable->insertType(
            "String",
            types.create<types::PointerTy>(types.create<types::CharTy>())
        );
        _scopeTable->insertType(
            "Int8", types.create<types::IntTy>(types::IntTy::Signed, 8)
        );
        _scopeTable->insertType(
            "Int16", types.create<types::IntTy>(types::IntTy::Signed, 16)
        );
        _scopeTable->insertType(
            "Int32", types.create<types::IntTy>(types::IntTy::Signed, 32)
        );
        _scopeTable->insertType(
            "Int64", types.create<types::IntTy>(types::IntTy::Signed, 64)
        );
        _scopeTable->insertType(
            "UInt8", types.create<types::IntTy>(types::IntTy::Unsigned, 8)
        );
        _scopeTable->insertType(
            "UInt16", types.create<types::IntTy>(types::IntTy::Unsigned, 16)
        );
        _scopeTable->insertType(
            "UInt32", types.create<types::IntTy>(types::IntTy::Unsigned, 32)
        );
        _scopeTable->insertType(
            "UInt64", types.create<types::IntTy>(types::IntTy::Unsigned, 64)
        );
    }

    void visitModuleDecl(ast::ModuleDecl *node)
    {
        for (auto *decl : node->getDecls()) {
            visit(decl);
        }
    }

    void visitTypeDecl(ast::TypeDecl *node)
    {
        _scopeTable->insertType(node->getName(), node->getType());
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
