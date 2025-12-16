#include "DeclImporter.hpp"

#include "AST/Attributes.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include "Basic/SourceLocation.hpp"

#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Casting.h>

namespace glu::clangimporter {

bool DeclImporter::VisitFunctionDecl(clang::FunctionDecl *funcDecl)
{
    // Skip function definitions (we only want declarations)
    if (funcDecl->hasBody()) {
        return true;
    }

    auto &astArena = _ctx.glu.getASTMemoryArena();
    auto &allocator = astArena.getAllocator();

    // Convert function type
    auto funcType = _typeConverter.convert(funcDecl->getType());
    auto *gluFuncType
        = llvm::dyn_cast_if_present<glu::types::FunctionTy>(funcType);
    if (!gluFuncType) {
        return true;
    }

    // Create parameter declarations
    llvm::SmallVector<glu::ast::ParamDecl *, 8> params;
    auto paramTypes = gluFuncType->getParameters();
    if (paramTypes.size() != funcDecl->getNumParams()) {
        // Mismatch in parameter count, skip this function
        return true;
    }
    for (size_t i = 0; i < funcDecl->getNumParams(); ++i) {
        auto *clangParam = funcDecl->getParamDecl(i);
        llvm::StringRef paramName = clangParam->getName();
        if (paramName.empty()) {
            paramName
                = copyString(("param" + llvm::Twine(i + 1)).str(), allocator);
        } else {
            paramName = copyString(paramName, allocator);
        }

        auto *paramDecl = astArena.create<glu::ast::ParamDecl>(
            SourceLocation::invalid, paramName, paramTypes[i], nullptr, nullptr
        );
        params.push_back(paramDecl);
    }

    // Create attributes
    llvm::SmallVector<glu::ast::Attribute *, 4> attrs;

    // Always add NoMangling attribute for C functions
    auto *noManglingAttr = astArena.create<glu::ast::Attribute>(
        glu::ast::AttributeKind::NoManglingKind, SourceLocation::invalid,
        nullptr
    );
    attrs.push_back(noManglingAttr);

    // Add CVariadic attribute if function is variadic
    if (funcDecl->isVariadic()) {
        auto *variadicAttr = astArena.create<glu::ast::Attribute>(
            glu::ast::AttributeKind::CVariadicKind, SourceLocation::invalid,
            nullptr
        );
        attrs.push_back(variadicAttr);
    }

    auto *attributeList = glu::ast::AttributeList::create(
        allocator, attrs, SourceLocation::invalid
    );

    // Create function declaration
    llvm::StringRef funcName = copyString(funcDecl->getName(), allocator);
    auto *gluFuncDecl = glu::ast::FunctionDecl::create(
        allocator, SourceLocation::invalid, nullptr, funcName, gluFuncType,
        params, nullptr, nullptr, glu::ast::Visibility::Public, attributeList
    );

    _ctx.importedDecls.push_back(gluFuncDecl);
    return true;
}

bool DeclImporter::VisitRecordDecl(clang::RecordDecl *recordDecl)
{
    // Skip forward declarations
    if (!recordDecl->isCompleteDefinition()) {
        return true;
    }

    // Skip anonymous structs for now
    if (!recordDecl->getIdentifier()) {
        return true;
    }

    auto &astArena = _ctx.glu.getASTMemoryArena();
    auto &allocator = astArena.getAllocator();

    // Create field declarations
    llvm::SmallVector<glu::ast::FieldDecl *, 16> fields;
    unsigned fieldIndex = 0;
    for (auto *field : recordDecl->fields()) {
        llvm::StringRef fieldName = field->getName();
        if (fieldName.empty()) {
            fieldName = copyString(
                ("field" + llvm::Twine(fieldIndex)).str(), allocator
            );
        } else {
            fieldName = copyString(fieldName, allocator);
        }

        auto fieldType = _typeConverter.convert(field->getType());
        if (!fieldType) {
            // Failed to convert field type, skip this record
            return true;
        }
        auto *fieldDecl = astArena.create<glu::ast::FieldDecl>(
            SourceLocation::invalid, fieldName, fieldType, nullptr, nullptr,
            glu::ast::Visibility::Public
        );
        fields.push_back(fieldDecl);
        fieldIndex++;
    }

    // Create struct declaration
    llvm::StringRef structName = copyString(recordDecl->getName(), allocator);
    auto *structDecl = glu::ast::StructDecl::create(
        allocator, _ctx.glu, SourceLocation::invalid, nullptr, structName,
        fields, nullptr, glu::ast::Visibility::Public, nullptr
    );

    _ctx.importedDecls.push_back(structDecl);
    _ctx.typeCache
        [_ctx.clang->getRecordType(recordDecl).getCanonicalType().getTypePtr()]
        = structDecl->getType();
    return true;
}

bool DeclImporter::VisitEnumDecl(clang::EnumDecl *enumDecl)
{
    // Skip forward declarations
    if (!enumDecl->isCompleteDefinition()) {
        return true;
    }

    // Skip anonymous enums for now
    if (!enumDecl->getIdentifier()) {
        return true;
    }

    auto &astArena = _ctx.glu.getASTMemoryArena();
    auto &allocator = astArena.getAllocator();

    // Create enum cases
    llvm::SmallVector<glu::ast::FieldDecl *, 16> cases;
    for (auto *enumConst : enumDecl->enumerators()) {
        llvm::StringRef caseName = copyString(enumConst->getName(), allocator);

        // Create a field for each enum case
        auto *caseDecl = astArena.create<glu::ast::FieldDecl>(
            SourceLocation::invalid, caseName, nullptr, nullptr, nullptr,
            glu::ast::Visibility::Public
        );
        cases.push_back(caseDecl);
    }

    // Get the underlying integer type
    auto underlyingType = _typeConverter.convert(enumDecl->getIntegerType());

    // Create enum declaration
    llvm::StringRef enumName = copyString(enumDecl->getName(), allocator);
    auto *gluEnumDecl = glu::ast::EnumDecl::create(
        allocator, _ctx.glu, SourceLocation::invalid, nullptr, enumName, cases,
        underlyingType, glu::ast::Visibility::Public, nullptr
    );

    _ctx.importedDecls.push_back(gluEnumDecl);
    _ctx.typeCache
        [_ctx.clang->getEnumType(enumDecl).getCanonicalType().getTypePtr()]
        = gluEnumDecl->getType();
    return true;
}

} // namespace glu::clangimporter
