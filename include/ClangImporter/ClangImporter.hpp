#ifndef GLU_CLANGIMPORTER_CLANGIMPORTER_HPP
#define GLU_CLANGIMPORTER_CLANGIMPORTER_HPP

#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"

#include <llvm/ADT/StringRef.h>
#include <vector>

namespace glu::clangimporter {

/// @brief Import a C header file and generate a Glu ModuleDecl
/// @param astContext The Glu AST context to use for creating nodes
/// @param headerPath The path to the header file to import
/// @param includePaths Additional include paths for finding headers
/// @return A ModuleDecl containing the imported declarations, or nullptr on
/// error
glu::ast::ModuleDecl *importHeader(
    glu::ast::ASTContext &astContext, llvm::StringRef headerPath,
    llvm::ArrayRef<std::string> cflags = {}
);

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_CLANGIMPORTER_HPP
