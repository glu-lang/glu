#ifndef GLU_CLANGIMPORTER_IMPORTERCONTEXT_HPP
#define GLU_CLANGIMPORTER_IMPORTERCONTEXT_HPP

#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include "Basic/SourceLocation.hpp"

#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>

#include <vector>

namespace glu::clangimporter {

/// ClangImporter Context (for importing Clang declarations to Glu AST)
struct ImporterContext {
    /// The Glu AST context
    glu::ast::ASTContext &glu;
    /// The Clang AST context
    clang::ASTContext *clang = nullptr;
    /// The list of imported Glu declarations
    std::vector<glu::ast::DeclBase *> importedDecls;
    /// Cache mapping Clang types to Glu types
    llvm::DenseMap<clang::Type const *, glu::types::TypeBase *> typeCache;
    /// Cache mapping file paths to Glu FileIDs for source location translation
    llvm::StringMap<glu::FileID> fileIdCache;

    ImporterContext(glu::ast::ASTContext &ast) : glu(ast) { }

    /// @brief Translate a Clang source location into a Glu source location.
    /// @param loc The Clang source location to translate.
    /// @return The corresponding Glu source location, or
    /// SourceLocation::invalid
    ///         if the input location is invalid or cannot be mapped to a Glu
    ///         file.
    SourceLocation translateSourceLocation(clang::SourceLocation loc);
};

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_IMPORTERCONTEXT_HPP
