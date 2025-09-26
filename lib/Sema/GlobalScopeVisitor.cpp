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
    scopeTable->insertItem(NAME, fn, ast::Visibility::Public);
#define BUILTIN_UNARY_OP(ID, NAME, RET, ARG)                                \
    fnType = typesArena.create<FunctionTy>(                                 \
        llvm::ArrayRef<TypeBase *>({ ARG }), RET                            \
    );                                                                      \
    fn = astArena.create<ast::FunctionDecl>(                                \
        SourceLocation::invalid, NAME, fnType,                              \
        llvm::ArrayRef<ast::ParamDecl *>({ astArena.create<ast::ParamDecl>( \
            SourceLocation::invalid, "value", ARG, nullptr                  \
        ) }),                                                               \
        ast::BuiltinKind::ID##Kind                                          \
    );                                                                      \
    scopeTable->insertItem(NAME, fn, ast::Visibility::Public);
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
            "Int", types.create<types::IntTy>(types::IntTy::Signed, 32),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Float", types.create<types::FloatTy>(types::FloatTy::FLOAT),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Double", types.create<types::FloatTy>(types::FloatTy::DOUBLE),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Float16", types.create<types::FloatTy>(types::FloatTy::HALF),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Float32", types.create<types::FloatTy>(types::FloatTy::FLOAT),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Float64", types.create<types::FloatTy>(types::FloatTy::DOUBLE),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Float80",
            types.create<types::FloatTy>(types::FloatTy::INTEL_LONG_DOUBLE),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Bool", types.create<types::BoolTy>(), ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Char", types.create<types::CharTy>(), ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Void", types.create<types::VoidTy>(), ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Null", types.create<types::NullTy>(), ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Int8", types.create<types::IntTy>(types::IntTy::Signed, 8),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Int16", types.create<types::IntTy>(types::IntTy::Signed, 16),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Int32", types.create<types::IntTy>(types::IntTy::Signed, 32),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Int64", types.create<types::IntTy>(types::IntTy::Signed, 64),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "Int128", types.create<types::IntTy>(types::IntTy::Signed, 128),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "UInt8", types.create<types::IntTy>(types::IntTy::Unsigned, 8),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "UInt16", types.create<types::IntTy>(types::IntTy::Unsigned, 16),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "UInt32", types.create<types::IntTy>(types::IntTy::Unsigned, 32),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "UInt64", types.create<types::IntTy>(types::IntTy::Unsigned, 64),
            ast::Visibility::Private
        );
        _scopeTable->insertType(
            "UInt128", types.create<types::IntTy>(types::IntTy::Unsigned, 128),
            ast::Visibility::Private
        );
        if (importManager) {
            _scopeTable->insertNamespace(
                "builtins",
                new (importManager->getScopeTableAllocator()) ScopeTable(
                    ScopeTable::NamespaceBuiltinsOverloadToken {},
                    _scopeTable->getModule()->getContext()
                ),
                ast::Visibility::Private
            );

            llvm::StringRef moduleName
                = _scopeTable->getModule()->getImportName();
            bool isInDefaultImports = false;
            for (auto component : llvm::make_range(
                     llvm::sys::path::begin(moduleName),
                     llvm::sys::path::end(moduleName)
                 )) {
                if (component == "defaultImports") {
                    isInDefaultImports = true;
                    break;
                }
            }

            if (!isInDefaultImports) {
                importManager->handleImport(
                    SourceLocation::invalid,
                    ast::ImportPath { llvm::ArrayRef<llvm::StringRef> {
                                          "defaultImports", "defaultImports" },
                                      llvm::ArrayRef<llvm::StringRef> { "*" } },
                    _scopeTable, ast::Visibility::Private
                );
            }
        }
    }

    void visitModuleDecl(ast::ModuleDecl *node)
    {
        for (auto *decl : node->getDecls()) {
            visit(decl);
        }
    }

    void visitTypeDecl(ast::TypeDecl *node)
    {
        _scopeTable->insertType(
            node->getName(), node->getType(), node->getVisibility()
        );
    }

    void visitFunctionDecl(ast::FunctionDecl *node)
    {
        _scopeTable->insertItem(node->getName(), node, node->getVisibility());
    }

    void visitVarLetDecl(ast::VarLetDecl *node)
    {
        _scopeTable->insertItem(node->getName(), node, node->getVisibility());
    }

    void visitImportDecl(ast::ImportDecl *node)
    {
        assert(_importManager && "ImportManager must be provided for imports");
        if (!_importManager->handleImport(
                node->getLocation(), node->getImportPath(), _scopeTable,
                node->getVisibility()
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
