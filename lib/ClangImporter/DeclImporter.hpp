#ifndef GLU_CLANGIMPORTER_DECLIMPORTER_HPP
#define GLU_CLANGIMPORTER_DECLIMPORTER_HPP

#include "ImporterContext.hpp"
#include "TypeConverter.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace glu::clangimporter {

/// AST visitor for importing Clang declarations
class DeclImporter : public clang::RecursiveASTVisitor<DeclImporter> {
    ImporterContext &_ctx;
    TypeConverter _typeConverter;

public:
    DeclImporter(ImporterContext &ctx) : _ctx(ctx), _typeConverter(ctx) { }

    bool VisitFunctionDecl(clang::FunctionDecl *funcDecl);
    bool VisitRecordDecl(clang::RecordDecl *recordDecl);
    bool VisitEnumDecl(clang::EnumDecl *enumDecl);
};

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_DECLIMPORTER_HPP
