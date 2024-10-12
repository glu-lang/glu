#include "SourceManager.hpp"

llvm::ErrorOr<glu::FileID> glu::SourceManager::loadFile(llvm::StringRef filePath
)
{
    llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> file
        = _vfs->openFileForRead(filePath);

    if (!file) {
        return file.getError();
    }

    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> buffer
        = (*file)->getBuffer(filePath);

    if (!buffer) {
        return buffer.getError();
    }

    ContentCache *cache = new ContentCache(filePath);
    cache->setBuffer(std::move(*buffer));

    auto fileName = (*file)->getName();

    if (!fileName) {
        return fileName.getError();
    }

    uint32_t newOffset = _sourceLocs.empty()
        ? 0
        : _sourceLocs.back().getOffset() + cache->getSize();

    SourceLocEntry entry(
        newOffset, FileInfo::get(SourceLocation(0), *cache, *fileName)
    );

    if (_sourceLocs.empty()) {
        _mainFile = FileID(_sourceLocs.size());
    }

    _sourceLocs.push_back(entry);

    return FileID(_sourceLocs.size());
}
