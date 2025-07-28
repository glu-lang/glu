#ifndef GLU_SEMA_BUILTIN_OPERATORS_HPP
#define GLU_SEMA_BUILTIN_OPERATORS_HPP

#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include "Basic/SourceLocation.hpp"
#include "ScopeTable.hpp"

#include <llvm/ADT/StringRef.h>

namespace glu::sema {

/// @brief Utility class for registering built-in arithmetic operators
class BuiltinOperators {
private:
    ast::ASTContext &_ctx;
    ScopeTable &_globalScope;

public:
    BuiltinOperators(ast::ASTContext &ctx, ScopeTable &globalScope) 
        : _ctx(ctx), _globalScope(globalScope) { }

    /// @brief Register all built-in arithmetic operators
    void registerBuiltinOperators()
    {
        auto &typesArena = _ctx.getTypesMemoryArena();

        // Get basic types - only integers for now
        auto *intTy = typesArena.create<types::IntTy>(types::IntTy::Signed, 32);

        // Register binary operators for integers only
        registerBinaryOperator("+", intTy, intTy, intTy);
        registerBinaryOperator("-", intTy, intTy, intTy);
        registerBinaryOperator("*", intTy, intTy, intTy);
        registerBinaryOperator("/", intTy, intTy, intTy);
    }

private:
    /// @brief Register a binary operator function
    void registerBinaryOperator(
        llvm::StringRef op, types::TypeBase *lhsTy, types::TypeBase *rhsTy,
        types::TypeBase *resultTy
    )
    {
        auto &typesArena = _ctx.getTypesMemoryArena();
        auto &astArena = _ctx.getASTMemoryArena();

        // Create function type
        std::vector<types::TypeBase *> paramTypes { lhsTy, rhsTy };
        auto *funcTy
            = typesArena.create<types::FunctionTy>(paramTypes, resultTy);

        // Create parameter declarations
        std::vector<ast::ParamDecl *> params;
        auto *lhsParam = astArena.create<ast::ParamDecl>(
            SourceLocation::invalid, "lhs", lhsTy, nullptr
        );
        auto *rhsParam = astArena.create<ast::ParamDecl>(
            SourceLocation::invalid, "rhs", rhsTy, nullptr
        );
        params.push_back(lhsParam);
        params.push_back(rhsParam);

        // Create function declaration (without body for intrinsics)
        auto *funcDecl = astArena.create<ast::FunctionDecl>(
            SourceLocation::invalid, nullptr, op, funcTy, params,
            nullptr
        );

        // Register in the global scope
        _globalScope.insertItem(op, funcDecl);
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_BUILTIN_OPERATORS_HPP
