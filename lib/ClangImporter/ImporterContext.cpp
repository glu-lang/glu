#include "ImporterContext.hpp"

#include "Basic/SourceManager.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

namespace glu::clangimporter {

SourceLocation ImporterContext::toSourceLocation(clang::SourceLocation loc)
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

    auto it = fileIdCache.find(filename);
    FileID gluFileId;
    if (it == fileIdCache.end()) {
        auto loaded = sm->loadFile(filename, true);
        if (!loaded) {
            return SourceLocation::invalid;
        }
        gluFileId = *loaded;
        fileIdCache.insert({ filename, gluFileId });
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
