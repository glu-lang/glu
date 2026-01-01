#ifndef GLU_CLANGIMPORTER_IMPORTACTION_HPP
#define GLU_CLANGIMPORTER_IMPORTACTION_HPP

#include "DeclImporter.hpp"
#include "ImporterContext.hpp"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <memory>

namespace glu::clangimporter {

/// AST consumer that drives the declaration import
class ImportASTConsumer : public clang::ASTConsumer {
    DeclImporter _importer;

public:
    ImportASTConsumer(ImporterContext &ctx) : _importer(ctx) { }

    void HandleTranslationUnit(clang::ASTContext &ctx) override
    {
        _importer.TraverseDecl(ctx.getTranslationUnitDecl());
    }
};

/// FrontendAction to capture imported declarations
class ImportAction : public clang::ASTFrontendAction {
    ImporterContext &_ctx;

public:
    ImportAction(ImporterContext &ctx) : _ctx(ctx) { }

    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override
    {
        _ctx.clang = &CI.getASTContext();
        return std::make_unique<ImportASTConsumer>(_ctx);
    }
};

/// Factory for creating ImportAction instances
class ImportActionFactory : public clang::tooling::FrontendActionFactory {
    ImporterContext &_ctx;

public:
    ImportActionFactory(ImporterContext &ctx) : _ctx(ctx) { }

    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<ImportAction>(_ctx);
    }
};

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_IMPORTACTION_HPP
