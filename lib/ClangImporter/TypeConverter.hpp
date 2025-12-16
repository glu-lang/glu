#ifndef GLU_CLANGIMPORTER_TYPECONVERTER_HPP
#define GLU_CLANGIMPORTER_TYPECONVERTER_HPP

#include "AST/Types.hpp"
#include "ImporterContext.hpp"

#include <clang/AST/Type.h>

namespace glu::clangimporter {

/// Type converter from Clang types to Glu types
class TypeConverter {
    ImporterContext &_ctx;

public:
    TypeConverter(ImporterContext &ctx) : _ctx(ctx) { }

    glu::types::TypeBase *convert(clang::QualType clangType);

private:
    glu::types::TypeBase *convertBuiltinType(clang::BuiltinType const *type);
    glu::types::TypeBase *
    convertFunctionType(clang::FunctionProtoType const *funcType);
};

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_TYPECONVERTER_HPP
