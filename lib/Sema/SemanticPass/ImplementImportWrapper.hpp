#ifndef GLU_SEMA_SEMANTICPASS_IMPLEMENTIMPORTWRAPPER_HPP
#define GLU_SEMA_SEMANTICPASS_IMPLEMENTIMPORTWRAPPER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/ImportManager.hpp"
#include "Sema/ScopeTable.hpp"

namespace glu::sema {

/// @brief Processes @implement imports by generating wrapper functions.
///
/// For each @implement import, this class generates a wrapper function with the
/// imported function's attributes. The wrapper calls the local implementation,
/// which will be resolved by constraint system later.
///
/// Example:
/// C header (myheader.h):
/// ```c
/// int computeValue(int);  // C function with @no_mangling
/// ```
/// Glu code:
/// ```glu
/// @implement import myheader::computeValue;
/// func computeValue(x: Int32) -> Int32 {
///     return x * 2;
/// }
/// ```
///
/// This generates a wrapper like:
/// ```glu
/// @no_mangling func computeValue(x: Int32) -> Int32 {
///     return computeValue(x);  // calls local impl (mangled differently)
/// }
/// ```
/// The correct mangling will automatically be applied based on the import.
class ImplementImportWrapper {
    ImportManager &_importManager;
    ScopeTable *_scopeTable;
    ast::ModuleDecl *_module;

public:
    ImplementImportWrapper(
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
    /// @brief Generate a wrapper function for an @implement import
    /// @param info The import information containing the imported function and
    /// local name
    /// @return The generated wrapper function declaration
    ast::FunctionDecl *generateWrapper(ImplementImportInfo const &info);
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_IMPLEMENTIMPORTWRAPPER_HPP
