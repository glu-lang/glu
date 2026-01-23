#ifndef GLU_SEMA_SEMANTICPASS_IMPLEMENTIMPORTCHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_IMPLEMENTIMPORTCHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/ImportManager.hpp"
#include "Sema/ScopeTable.hpp"

namespace glu::sema {

/// @brief Processes @implement imports by generating wrapper functions.
///
/// For each @implement import, this class:
/// 1. Finds the local function implementation with matching name
/// 2. Generates a wrapper function with the imported function's attributes
/// 3. The wrapper calls the local implementation
///
/// Example:
/// @implement import myheader::computeValue;  // imports int computeValue(int)
///                                            // from C header with
///                                            @no_mangling
/// func computeValue(x: Int32) -> Int32 {     // local implementation
///     return x * 2;
/// }
///
/// This generates a wrapper like:
/// @no_mangling func computeValue(x: Int32) -> Int32 {
///     return computeValue(x);  // calls local impl (mangled differently)
/// }
class ImplementImportChecker {
    ImportManager &_importManager;
    ScopeTable *_scopeTable;
    ast::ModuleDecl *_module;

public:
    ImplementImportChecker(
        ImportManager &importManager, ScopeTable *scopeTable,
        ast::ModuleDecl *module
    )
        : _importManager(importManager)
        , _scopeTable(scopeTable)
        , _module(module)
    {
    }

    /// @brief Process all @implement imports and generate wrapper functions
    void process();

private:
    /// @brief Find a local function implementation matching the imported
    /// prototype
    /// @param name The function name to search for
    /// @param type The function type that must match
    /// @return The matching local function, or nullptr
    ast::FunctionDecl *generateWrapper(ImplementImportInfo const &info);
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_IMPLEMENTIMPORTCHECKER_HPP
