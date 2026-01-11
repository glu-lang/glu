#include "ClangImporter/ClangImporter.hpp"
#include "AST/Decls.hpp"
#include "Basic/SourceLocation.hpp"
#include "ImportAction.hpp"
#include "ImporterContext.hpp"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::clangimporter {

/// Main entry point for importing a C header file
glu::ast::ModuleDecl *importHeader(
    glu::ast::ASTContext &astContext, llvm::StringRef headerPath,
    llvm::ArrayRef<std::string> cflags
)
{
    // Check if file exists
    if (!llvm::sys::fs::exists(headerPath)) {
        llvm::errs() << "Failed to open file: " << headerPath << "\n";
        return nullptr;
    }

    // Load compilation database from arguments, one argument per line
    std::string commandData = "-xc\n";
    for (auto const &arg : cflags) {
        commandData += arg + "\n";
    }

    std::string errorMsg;
    // TODO: Fix directory to be the directory of the importing file?
    auto directory = llvm::sys::path::parent_path(headerPath);
    auto compileDB = clang::tooling::FixedCompilationDatabase::loadFromBuffer(
        directory, commandData, errorMsg
    );

    if (!compileDB) {
        llvm::errs() << "Failed to create compilation database: " << errorMsg
                     << "\n";
        return nullptr;
    }

    // Create ClangTool
    clang::tooling::ClangTool tool(*compileDB, { headerPath.str() });

    // Create import context
    ImporterContext importCtx(astContext);

    // Run the tool with our custom action factory
    ImportActionFactory factory(importCtx);
    if (tool.run(&factory) != 0) {
        llvm::errs() << "Failed to parse file: " << headerPath << "\n";
        return nullptr;
    }

    SourceLocation moduleLoc = SourceLocation::invalid;
    if (auto *sm = astContext.getSourceManager()) {
        if (auto fid = sm->loadFile(headerPath, true)) {
            moduleLoc = sm->getLocForStartOfFile(*fid);
        }
    }

    // Create module declaration
    auto *moduleDecl = glu::ast::ModuleDecl::create(
        astContext.getASTMemoryArena().getAllocator(), moduleLoc,
        importCtx.importedDecls, &astContext
    );

    return moduleDecl;
}

} // namespace glu::clangimporter
