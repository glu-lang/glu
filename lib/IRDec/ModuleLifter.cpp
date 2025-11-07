#include "ModuleLifter.hpp"
#include "AST/Exprs.hpp"
#include "DITypeLifter.hpp"
#include "TypeLifter.hpp"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IntrinsicInst.h>

namespace glu::irdec {

class ModuleLifter {
    glu::ast::ASTContext &_astContext;
    llvm::Module *_llvmModule;

    /// @brief Extract parameter names from debug variables in a function
    /// @param func The function to extract parameter names from
    /// @return A map from argument index to parameter name
    llvm::DenseMap<unsigned, llvm::StringRef>
    extractParameterNames(llvm::Function const &func)
    {
        llvm::DenseMap<unsigned, llvm::StringRef> paramNames;

        // In LLVM, we can find debug info for function arguments by using
        // llvm::findDbgDeclares or by iterating through the retained nodes
        // in the function's subprogram
        if (auto *sp = func.getSubprogram()) {
            auto retainedNodes = sp->getRetainedNodes();
            for (auto *node : retainedNodes) {
                if (auto *localVar
                    = llvm::dyn_cast<llvm::DILocalVariable>(node)) {
                    unsigned argNum = localVar->getArg();
                    if (argNum > 0) {
                        // Argument indices are 1-based, we need 0-based
                        unsigned index = argNum - 1;
                        paramNames[index] = localVar->getName();
                    }
                }
            }
        }

        return paramNames;
    }

    /// @brief Generate parameter declarations with default names
    /// @param types The parameter types
    /// @return A vector of parameter declarations
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

    /// @brief Generate parameter declarations with names from debug info
    /// @param types The parameter types
    /// @param paramNames Map from argument index to parameter name
    /// @return A vector of parameter declarations
    llvm::SmallVector<glu::ast::ParamDecl *, 4> generateParamsDecls(
        llvm::ArrayRef<glu::types::TypeBase *> types,
        llvm::DenseMap<unsigned, llvm::StringRef> const &paramNames
    )
    {
        llvm::SmallVector<glu::ast::ParamDecl *, 4> params;

        for (size_t i = 0; i < types.size(); ++i) {
            llvm::StringRef paramName;
            std::string defaultName;

            // Use debug info name if available, otherwise use default name
            if (auto it = paramNames.find(i); it != paramNames.end()) {
                paramName = it->second;
            } else {
                defaultName = "param" + std::to_string(i);
                paramName = defaultName;
            }

            auto paramDecl
                = _astContext.getASTMemoryArena().create<glu::ast::ParamDecl>(
                    SourceLocation::invalid,
                    copyString(
                        paramName,
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

                // Extract parameter names from debug info
                auto paramNames = extractParameterNames(func);

                auto funcDecl = astArena.create<glu::ast::FunctionDecl>(
                    SourceLocation::invalid, nullptr, funcName, funcType,
                    generateParamsDecls(funcType->getParameters(), paramNames),
                    nullptr, nullptr, glu::ast::Visibility::Public, attributes
                );
                decls.push_back(funcDecl);
            }
        }

        for (auto decl : diTypeLifter.getDeclBindings()) {
            decls.push_back(decl.second);
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
