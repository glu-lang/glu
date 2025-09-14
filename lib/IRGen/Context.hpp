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
};

} // namespace glu::irgen

#endif // GLU_IRGEN_CONTEXT_HPP
