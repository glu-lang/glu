#include "ModuleLifter.hpp"
#include "AST/Exprs.hpp"

#include <llvm/IR/CallingConv.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugProgramInstruction.h>
#include <llvm/IR/Function.h>

namespace glu::irdec {

class ModuleLifter {
    ModuleLiftingContext _ctx;
    glu::ast::ASTContext &_astContext;
    llvm::Module *_llvmModule;

    /// @brief Try to add parameter name from a local variable if it's a
    /// parameter
    /// @param localVar The local variable to check
    /// @param paramNames The map to add the parameter name to
    static void tryAddParameterName(
        llvm::DILocalVariable const *localVar,
        llvm::DenseMap<unsigned, llvm::StringRef> &paramNames
    )
    {
        unsigned argNum = localVar->getArg();
        if (argNum > 0) {
            // Argument indices are 1-based, we need 0-based
            unsigned index = argNum - 1;
            paramNames[index] = localVar->getName();
        }
    }

    /// @brief Extract parameter names from retained nodes in subprogram
    /// @param sp The subprogram to extract parameter names from
    /// @param paramNames The map to add parameter names to
    static void extractParameterNamesFromRetainedNodes(
        llvm::DISubprogram const *sp,
        llvm::DenseMap<unsigned, llvm::StringRef> &paramNames
    )
    {
        auto retainedNodes = sp->getRetainedNodes();
        for (auto *node : retainedNodes) {
            if (auto *localVar = llvm::dyn_cast<llvm::DILocalVariable>(node)) {
                tryAddParameterName(localVar, paramNames);
            }
        }
    }

    /// @brief Extract parameter names from debug records in function body
    /// @param func The function to extract parameter names from
    /// @param paramNames The map to add parameter names to
    static void extractParameterNamesFromDebugRecords(
        llvm::Function const &func,
        llvm::DenseMap<unsigned, llvm::StringRef> &paramNames
    )
    {
        for (auto &bb : func) {
            for (auto &inst : bb) {
                // Check for new-style debug records (LLVM 19+)
                for (auto &dbgRecord : inst.getDbgRecordRange()) {
                    if (auto *dbgVarRecord
                        = llvm::dyn_cast<llvm::DbgVariableRecord>(&dbgRecord)) {
                        if (dbgVarRecord->getType()
                            == llvm::DbgVariableRecord::LocationType::Declare) {
                            if (auto *localVar = dbgVarRecord->getVariable()) {
                                tryAddParameterName(localVar, paramNames);
                            }
                        }
                    }
                }
            }
        }
    }

    /// @brief Extract parameter names from debug variables in a function
    /// @param func The function to extract parameter names from
    /// @return A map from argument index to parameter name
    llvm::DenseMap<unsigned, llvm::StringRef>
    extractParameterNames(llvm::Function const &func)
    {
        llvm::DenseMap<unsigned, llvm::StringRef> paramNames;

        // First try retained nodes (may be populated in some cases)
        if (auto *sp = func.getSubprogram()) {
            extractParameterNamesFromRetainedNodes(sp, paramNames);
        }

        // If retained nodes didn't give us all parameter names,
        // look for debug records in the function body
        extractParameterNamesFromDebugRecords(func, paramNames);

        return paramNames;
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
                defaultName = "param" + std::to_string(i + 1);
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
        : _ctx(astContext), _astContext(astContext), _llvmModule(llvmModule)
    {
    }

    ~ModuleLifter() = default;

    glu::ast::ModuleDecl *detectExternalFunctions()
    {
        auto &astArena = _astContext.getASTMemoryArena();

        for (auto &func : _llvmModule->functions()) {
            if (!func.isDeclaration()
                && func.getLinkage() == llvm::Function::ExternalLinkage) {
                types::Ty type;
                llvm::StringRef funcName;
                if (auto subprogram = func.getSubprogram()) {
                    type = lift(subprogram->getType(), _ctx);
                    funcName = copyString(
                        subprogram->getName(),
                        _astContext.getASTMemoryArena().getAllocator()
                    );
                } else {
                    type = lift(func.getFunctionType(), _ctx);
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
                // Add calling convention attribute for non-default calling
                // conventions
                auto cc = func.getCallingConv();
                if (cc != llvm::CallingConv::C) {
                    auto *ccParam = astArena.create<ast::LiteralExpr>(
                        llvm::APInt(32, cc), nullptr, SourceLocation::invalid
                    );
                    auto *attr = astArena.create<ast::Attribute>(
                        ast::AttributeKind::CallingConventionKind,
                        SourceLocation::invalid, ccParam
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
                _ctx.addToNamespace(
                    func.getSubprogram() ? func.getSubprogram()->getScope()
                                         : nullptr,
                    funcDecl
                );
            }
        }
        return _astContext.getASTMemoryArena()
            .create<glu::ast::ModuleDecl>(
                SourceLocation::invalid, std::move(_ctx.rootDecls), &_astContext
            )
            ->markAsIRDecModule();
    }
};

glu::ast::DeclBase *ModuleLiftingContext::addToNamespace(
    llvm::DIScope const *dins, glu::ast::DeclBase *content
)
{
    // We cannot cache namespaces as we cannot resize them once created
    // Luckily, using multiple namespace declarations with the same name is
    // supported
    // We support all DIScope kinds that can contain declarations, so C++
    // structures etc are considered namespaces in Glu
    if (!dins || llvm::isa<llvm::DIFile>(dins)) {
        // If dins is null, we are at the root namespace, add to rootDecls
        rootDecls.push_back(content);
        return content;
    }
    auto &astArena = ast.getASTMemoryArena();
    return addToNamespace(
        dins->getScope(),
        astArena.create<glu::ast::NamespaceDecl>(
            SourceLocation::invalid, nullptr,
            copyString(dins->getName(), astArena.getAllocator()),
            llvm::ArrayRef<glu::ast::DeclBase *> { content },
            glu::ast::Visibility::Public
        )
    );
}

glu::ast::ModuleDecl *
liftModule(glu::ast::ASTContext &astContext, llvm::Module *llvmModule)
{
    ModuleLifter lifter(astContext, llvmModule);
    return lifter.detectExternalFunctions();
}

} // namespace glu::irdec
