#include "Basic/SourceManager.hpp"

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

    auto fileName = (*file)->getName();

    if (!fileName) {
        return fileName.getError();
    }

    uint32_t newOffset = _fileLocEntries.empty()
        ? 0
        : _fileLocEntries.back().getOffset() + (*buffer)->getBufferSize();

    _fileLocEntries.emplace_back(
        newOffset, std::move(*buffer), SourceLocation(0), *fileName
    );

    if (_fileLocEntries.empty()) {
        _mainFile = FileID(_fileLocEntries.size());
    }

    return FileID(_fileLocEntries.size());
}
