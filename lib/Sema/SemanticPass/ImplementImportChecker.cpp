#include "ImplementImportChecker.hpp"

#include "AST/Decl/FunctionDecl.hpp"
#include "AST/Decl/ImportDecl.hpp"
#include "AST/Decl/ParamDecl.hpp"
#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"
#include "AST/Types/VoidTy.hpp"

namespace glu::sema {

void ImplementImportChecker::process()
{
    auto const &implementImports = _importManager.getImplementImports();
    if (implementImports.empty()) {
        return;
    }

    for (auto const &info : implementImports) {
        if (info.intoScope != _scopeTable) {
            // This import is not for the current module
            continue;
        }
        // Generate a wrapper function that has the imported function's
        // attributes and calls the local implementation.
        // The wrapper contains unresolved references that Sema will resolve.
        auto *wrapper = generateWrapper(info);
        if (wrapper) {
            _scopeTable->addSyntheticFunction(wrapper);
        }
    }
}

ast::FunctionDecl *
ImplementImportChecker::generateWrapper(ImplementImportInfo const &info)
{
    auto *ctx = _module->getContext();
    auto &astArena = ctx->getASTMemoryArena();
    auto &allocator = astArena.getAllocator();
    auto *importedFunc = info.importedFunc;

    // Copy parameters from the imported function
    auto importedParams = importedFunc->getParams();
    llvm::SmallVector<ast::ParamDecl *, 4> newParams;

    for (auto *param : importedParams) {
        // Create new parameter declarations for the wrapper
        // They should have the same name and type as the imported function
        auto *newParam = astArena.create<ast::ParamDecl>(
            param->getLocation(), param->getName(), param->getType(),
            nullptr // no default value
        );
        newParams.push_back(newParam);
    }

    // Build the call arguments from the parameters (unresolved)
    llvm::SmallVector<ast::ExprBase *, 4> callArgs;
    for (auto *param : newParams) {
        // Create an unresolved RefExpr for each parameter
        // Sema will resolve these to the actual parameter declarations
        ast::NamespaceIdentifier ident { {}, param->getName() };
        auto *paramRef = ast::RefExpr::create(
            allocator, param->getLocation(), ident, nullptr
        );
        callArgs.push_back(paramRef);
    }

    // Create an unresolved RefExpr to the local implementation
    // Sema will resolve this to the actual local function
    ast::NamespaceIdentifier localIdent { {}, info.effectiveName };
    auto *localRef = ast::RefExpr::create(
        allocator, importedFunc->getLocation(), localIdent, nullptr
    );

    // Create the call expression to the local implementation
    auto *callExpr = ast::CallExpr::create(
        allocator, importedFunc->getLocation(), localRef, callArgs
    );

    // Create the return statement (or void return for void functions)
    llvm::SmallVector<ast::StmtBase *, 2> stmts;
    auto *returnType = importedFunc->getType()->getReturnType();

    if (llvm::isa<types::VoidTy>(returnType)) {
        // For void functions: call, then return
        // Wrap call in an ExpressionStmt
        auto *exprStmt = astArena.create<ast::ExpressionStmt>(
            importedFunc->getLocation(), callExpr
        );
        stmts.push_back(exprStmt);
        auto *returnStmt = astArena.create<ast::ReturnStmt>(
            importedFunc->getLocation(), nullptr
        );
        stmts.push_back(returnStmt);
    } else {
        // For non-void functions: return the call result
        auto *returnStmt = astArena.create<ast::ReturnStmt>(
            importedFunc->getLocation(), callExpr
        );
        stmts.push_back(returnStmt);
    }

    // Create the body block
    auto *body = ast::CompoundStmt::create(
        allocator, importedFunc->getLocation(), stmts
    );

    // Copy the attributes from the imported function
    // These attributes contain the linkage information (like @no_mangling)
    ast::AttributeList *attrs = importedFunc->getAttributes();

    // Create the wrapper function with the imported function's name
    // and attributes, but with a body that calls the local implementation
    // Set the parent to the module so GILGen can access the context
    auto *wrapper = ast::FunctionDecl::create(
        allocator, importedFunc->getLocation(),
        // parent use the same parent as imported for correct mangling
        importedFunc->getParent(),
        info.selectorName, // Use the original selector name for linkage
        importedFunc->getType(), newParams, body,
        nullptr, // no template params
        ast::Visibility::Public, attrs
    );

    return wrapper;
}

} // namespace glu::sema
