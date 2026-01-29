#ifndef GLU_CLANGIMPORTER_TYPECONVERTER_HPP
#define GLU_CLANGIMPORTER_TYPECONVERTER_HPP

#include "ImporterContext.hpp"

#include <llvm/ADT/StringRef.h>

namespace clang {
class EnumDecl;
class RecordDecl;
class TypedefNameDecl;
} // namespace clang

#include <clang/AST/Type.h>

namespace glu::clangimporter {

/// Type converter from Clang types to Glu types
class TypeConverter {
    ImporterContext &_ctx;

public:
    TypeConverter(ImporterContext &ctx) : _ctx(ctx) { }

    glu::types::TypeBase *convert(clang::QualType clangType);
    glu::types::TypeBase *importRecordDecl(
        clang::RecordDecl *recordDecl, bool allowIncomplete,
        llvm::StringRef forcedName = {}
    );
    glu::types::TypeBase *importEnumDecl(
        clang::EnumDecl *enumDecl, bool allowIncomplete,
        llvm::StringRef forcedName = {}
    );
    glu::types::TypeBase *
    importTypedefDecl(clang::TypedefNameDecl *typedefDecl);

private:
    glu::types::TypeBase *convertBuiltinType(clang::BuiltinType const *type);
    glu::types::TypeBase *convertRecordType(clang::RecordType const *type);
    glu::types::TypeBase *convertEnumType(clang::EnumType const *type);
    glu::types::TypeBase *
    convertFunctionType(clang::FunctionProtoType const *funcType);
};

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_TYPECONVERTER_HPP
