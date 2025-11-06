#include "ModuleLifter.hpp"
#include "AST/Exprs.hpp"
#include "DITypeLifter.hpp"
#include "TypeLifter.hpp"

#include <llvm/IR/Function.h>

namespace glu::irdec {

class ModuleLifter {
    glu::ast::ASTContext &_astContext;
    llvm::Module *_llvmModule;

    llvm::SmallVector<glu::ast::ParamDecl *, 4>
    generateParamsDecls(llvm::ArrayRef<glu::types::TypeBase *> types)
    {
        llvm::SmallVector<glu::ast::ParamDecl *, 4> params;

        for (size_t i = 0; i < types.size(); ++i) {
            auto paramDecl
                = _astContext.getASTMemoryArena().create<glu::ast::ParamDecl>(
                    SourceLocation::invalid,
                    copyString(
                        "param" + std::to_string(i),
                        _astContext.getASTMemoryArena().getAllocator()
                    ),
                    types[i], nullptr
                );
            params.push_back(paramDecl);
        }
        return params;
    }

public:
    ModuleLifter(glu::ast::ASTContext &astContext, llvm::Module *llvmModule)
        : _astContext(astContext), _llvmModule(llvmModule)
    {
    }

    ~ModuleLifter() = default;

    glu::ast::ModuleDecl *detectExternalFunctions()
    {
        DITypeLifter diTypeLifter(_astContext);
        TypeLifter typeLifter(_astContext);
        std::vector<glu::ast::DeclBase *> decls;
        auto &astArena = _astContext.getASTMemoryArena();

        for (auto &func : _llvmModule->functions()) {
            if (!func.isDeclaration()
                && func.getLinkage() == llvm::Function::ExternalLinkage) {
                types::Ty type;
                llvm::StringRef funcName;
                if (auto subprogram = func.getSubprogram()) {
                    type = diTypeLifter.lift(subprogram->getType());
                    funcName = copyString(
                        subprogram->getName(),
                        _astContext.getASTMemoryArena().getAllocator()
                    );
                } else {
                    type = typeLifter.lift(func.getFunctionType());
                    funcName = copyString(
                        func.getName(),
                        _astContext.getASTMemoryArena().getAllocator()
                    );
                }
                auto funcType
                    = llvm::dyn_cast_if_present<glu::types::FunctionTy>(type);
                if (!funcType)
                    continue;
                llvm::SmallVector<ast::Attribute *, 4> attrs;
                if (funcName != func.getName()) {
                    auto linkageName = copyString(
                        func.getName(),
                        _astContext.getASTMemoryArena().getAllocator()
                    );
                    auto *attr = astArena.create<ast::Attribute>(
                        ast::AttributeKind::LinkageNameKind,
                        SourceLocation::invalid,
                        astArena.create<ast::LiteralExpr>(
                            linkageName, nullptr, SourceLocation::invalid
                        )
                    );
                    attrs.push_back(attr);
                } else {
                    auto *attr = astArena.create<ast::Attribute>(
                        ast::AttributeKind::NoManglingKind,
                        SourceLocation::invalid, nullptr
                    );
                    attrs.push_back(attr);
                }
                if (func.isVarArg()) {
                    auto *attr = astArena.create<ast::Attribute>(
                        ast::AttributeKind::CVariadicKind,
                        SourceLocation::invalid, nullptr
                    );
                    attrs.push_back(attr);
                }
                ast::AttributeList *attributes
                    = astArena.create<ast::AttributeList>(
                        attrs, SourceLocation::invalid
                    );
                auto funcDecl = astArena.create<glu::ast::FunctionDecl>(
                    SourceLocation::invalid, nullptr, funcName, funcType,
                    generateParamsDecls(funcType->getParameters()), nullptr,
                    nullptr, glu::ast::Visibility::Public, attributes
                );
                decls.push_back(funcDecl);
            }
        }

        for (auto decl : diTypeLifter.getDeclBindings()) {
            decls.push_back(decl.second);
        }
        // FIXME: Do the same with typeLifter!!!
        return _astContext.getASTMemoryArena().create<glu::ast::ModuleDecl>(
            SourceLocation::invalid, std::move(decls), &_astContext
        );
    }
};

glu::ast::ModuleDecl *
liftModule(glu::ast::ASTContext &astContext, llvm::Module *llvmModule)
{
    ModuleLifter lifter(astContext, llvmModule);
    return lifter.detectExternalFunctions();
}

} // namespace glu::irdec
