#include "ImporterContext.hpp"

#include "Basic/SourceManager.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/FileSystem.h>

namespace glu::clangimporter {

SourceLocation
ImporterContext::translateSourceLocation(clang::SourceLocation loc)
{
    auto *sm = glu.getSourceManager();
    if (!sm || !clang || loc.isInvalid()) {
        return SourceLocation::invalid;
    }

    auto &clangSM = clang->getSourceManager();
    auto spellingLoc = clangSM.getSpellingLoc(loc);
    if (spellingLoc.isInvalid()) {
        return SourceLocation::invalid;
    }

    llvm::StringRef filename = clangSM.getFilename(spellingLoc);

    if (filename.empty() || filename.starts_with("<")) {
        return SourceLocation::invalid;
    }

    // Normalize to absolute path to ensure consistent cache keys
    llvm::SmallString<256> absPath(filename);
    if (llvm::sys::fs::make_absolute(absPath)) {
        return SourceLocation::invalid;
    }

    auto it = fileIdCache.find(absPath);
    FileID gluFileId;
    if (it == fileIdCache.end()) {
        auto loaded = sm->loadFile(absPath, true);
        if (!loaded) {
            return SourceLocation::invalid;
        }
        gluFileId = *loaded;
        fileIdCache.insert({ absPath, gluFileId });
    } else {
        gluFileId = it->second;
    }

    auto baseLoc = sm->getLocForStartOfFile(gluFileId);
    if (baseLoc.isInvalid()) {
        return SourceLocation::invalid;
    }

    auto offset = clangSM.getFileOffset(spellingLoc);
    return SourceLocation(baseLoc.getOffset() + offset);
}

} // namespace glu::clangimporter
