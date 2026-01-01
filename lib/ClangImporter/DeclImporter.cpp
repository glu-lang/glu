#include "DeclImporter.hpp"

#include "AST/Attributes.hpp"
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
    // Skip static functions (no external linkage)
    if (funcDecl->getStorageClass() == clang::SC_Static) {
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
    _typeConverter.importRecordDecl(recordDecl, false);
    return true;
}

bool DeclImporter::VisitEnumDecl(clang::EnumDecl *enumDecl)
{
    _typeConverter.importEnumDecl(enumDecl, false);
    return true;
}

} // namespace glu::clangimporter
