#ifndef GLU_IRGEN_CONTEXT_HPP
#define GLU_IRGEN_CONTEXT_HPP

#include <Basic/SourceManager.hpp>

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace glu::irgen {

/// @brief The context/builder for the IR code generation.
struct Context {
    llvm::Module &outModule;
    llvm::LLVMContext &ctx;
    llvm::DIBuilder dib;
    SourceManager *sm = nullptr;

public:
    Context(llvm::Module &module, SourceManager *sm)
        : outModule(module), ctx(module.getContext()), dib(module), sm(sm)
    {
    }

    llvm::DIFile *createDIFile(SourceLocation loc)
    {
        if (sm && loc.isValid()) {
            auto path = sm->getBufferName(loc);
            return dib.createFile(
                llvm::sys::path::filename(path),
                llvm::sys::path::parent_path(path)
            );
        }
        return nullptr;
    }

    llvm::DIScope *getScopeForDecl(ast::DeclBase *decl)
    {
        if (!decl) {
            return nullptr;
        }

        auto *ns = llvm::dyn_cast<ast::NamespaceDecl>(decl);
        auto *parent = llvm::cast_if_present<ast::DeclBase>(decl->getParent());
        if (!ns) {
            return getScopeForDecl(parent);
        }
        return dib.createNameSpace(
            getScopeForDecl(parent), ns->getName(), false
        );
    }
};

} // namespace glu::irgen

#endif // GLU_IRGEN_CONTEXT_HPP
