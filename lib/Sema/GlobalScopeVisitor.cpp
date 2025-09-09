#include "ImportManager.hpp"
#include "ScopeTable.hpp"

#include "AST/ASTContext.hpp"
#include "AST/ASTVisitor.hpp"

namespace glu::sema {

void registerBinaryBuiltinsOP(ScopeTable *scopeTable, ast::ASTContext *ctx)
{
    using namespace glu::types;
    ast::FunctionDecl *fn = nullptr;
    FunctionTy *fnType = nullptr;

    auto &astArena = ctx->getASTMemoryArena();
    auto &typesArena = ctx->getTypesMemoryArena();

#define TYPE(NAME, ...) typesArena.create<NAME##Ty>(__VA_ARGS__)
#define BUILTIN_BINARY_OP(ID, NAME, RET, ARG1, ARG2)            \
    fnType = typesArena.create<FunctionTy>(                     \
        llvm::ArrayRef<TypeBase *>({ ARG1, ARG2 }), RET         \
    );                                                          \
    fn = astArena.create<ast::FunctionDecl>(                    \
        SourceLocation::invalid, NAME, fnType,                  \
        llvm::ArrayRef<ast::ParamDecl *>(                       \
            { astArena.create<ast::ParamDecl>(                  \
                  SourceLocation::invalid, "lhs", ARG1, nullptr \
              ),                                                \
              astArena.create<ast::ParamDecl>(                  \
                  SourceLocation::invalid, "rhs", ARG2, nullptr \
              ) }                                               \
        ),                                                      \
        ast::BuiltinKind::ID##Kind                              \
    );                                                          \
    scopeTable->insertItem(NAME, fn);
#include "AST/Decl/Builtins.def"
}

ScopeTable::ScopeTable(NamespaceBuiltinsOverloadToken, ast::ASTContext *context)
    : _parent(nullptr), _node(nullptr)
{
    registerBinaryBuiltinsOP(this, context);
}

class GlobalScopeVisitor
    : public glu::ast::ASTVisitor<GlobalScopeVisitor, void> {
    /// @brief The scope table we are populating.
    ScopeTable *_scopeTable;
    ImportManager *_importManager;

public:
    /// @brief Constructor.
    /// @param scopeTable The scope table to populate.
    GlobalScopeVisitor(ScopeTable *scopeTable, ImportManager *importManager)
        : _scopeTable(scopeTable), _importManager(importManager)
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
        _scopeTable->insertNamespace(
            "builtins",
            new (importManager->getScopeTableAllocator()) ScopeTable(
                ScopeTable::NamespaceBuiltinsOverloadToken {},
                _scopeTable->getModule()->getContext()
            )
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

    void visitImportDecl(ast::ImportDecl *node)
    {
        assert(_importManager && "ImportManager must be provided for imports");
        if (!_importManager->handleImport(
                node->getLocation(), node->getImportPath(), _scopeTable
            )) {
            // Import failed, report error.
            _importManager->getDiagnosticManager().error(
                node->getLocation(), "Import failed"
            );
        }
    }
};

ScopeTable::ScopeTable(ast::ModuleDecl *node, ImportManager *importManager)
    : _parent(nullptr), _node(node)
{
    assert(node && "Node must be provided for global scope (ModuleDecl)");
    GlobalScopeVisitor(this, importManager).visit(node);
}

} // namespace glu::sema
