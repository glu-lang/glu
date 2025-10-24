#include "ModuleLifter.hpp"
#include "DITypeLifter.hpp"
#include "GILGen/GILGen.hpp"
#include "llvm/IR/Function.h"

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
        DITypeLifter typeLifter(_astContext);
        std::vector<glu::ast::DeclBase *> decls;

        for (auto &func : _llvmModule->functions()) {
            if (!func.isDeclaration()
                && func.getLinkage() == llvm::Function::ExternalLinkage) {
                auto subProgram = func.getSubprogram();
                if (!subProgram)
                    continue;
                auto type = typeLifter.lift(subProgram->getType());
                if (auto funcType
                    = llvm::dyn_cast_if_present<glu::types::FunctionTy>(type)) {
                    auto funcDecl
                        = _astContext.getASTMemoryArena()
                              .create<glu::ast::FunctionDecl>(
                                  SourceLocation::invalid, nullptr,
                                  func.getName(), funcType,
                                  generateParamsDecls(funcType->getParameters()
                                  ),
                                  nullptr, glu::ast::Visibility::Public
                              );
                    decls.push_back(funcDecl);
                }
            }
        }

        for (auto decl : typeLifter.getDeclBindings()) {
            decls.push_back(decl.second);
        }
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
