#include "Basic/SourceManager.hpp"
#include "Basic/SourceLocation.hpp"

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

    return llvm::ErrorOr<glu::FileID>(glu::FileID(_fileLocEntries.size()));
}

glu::SourceLocation glu::SourceManager::getLocForStartOfFile(FileID fileID
) const
{
    if (fileID == FileID(0)) {
        return SourceLocation(0);
    }

    return SourceLocation(_fileLocEntries[fileID._id - 1].getOffset());
}

glu::SourceLocation glu::SourceManager::getLocForEndOfFile(FileID fileID) const
{
    if (fileID == FileID(0)) {
        return SourceLocation(0);
    }

    return SourceLocation(
        _fileLocEntries[fileID._id - 1].getOffset()
        + _fileLocEntries[fileID._id - 1].getSize()
    );
}

char const *glu::SourceManager::getCharacterData(SourceLocation loc) const
{
    if (loc == SourceLocation(0)) {
        return nullptr;
    }

    auto const &fileLocEntry
        = _fileLocEntries[glu::SourceManager::getFileID(loc)._id - 1];
    auto buffer = fileLocEntry.getBufferDataIfLoaded();

    if (!buffer) {
        return nullptr;
    }

    return buffer->data() + loc._offset;
}

bool glu::SourceManager::isOffsetInFileID(glu::FileID fid, unsigned offset)
    const
{
    auto const &fileLocEntry = _fileLocEntries[fid._id];

    if (fileLocEntry.getOffset() > offset) {
        return false;
    }

    if (fid._id + 1 == _fileLocEntries.size()) {
        return offset < _nextOffset;
    }

    return offset < _fileLocEntries[fid._id + 1].getOffset();
}

glu::FileID glu::SourceManager::getFileID(unsigned offset)
{
    assert(offset < _nextOffset && "Offset is out of bounds.");

    if (isOffsetInFileID(_lastLookupFileID, offset)) {
        return _lastLookupFileID;
    }

    if (!offset) {
        return FileID(0);
    }

    unsigned lessIndex = 0;
    unsigned maxIndex = _fileLocEntries.size();

    if (_lastLookupFileID._id >= 0) {
        if (_fileLocEntries[_lastLookupFileID._id].getOffset() < offset) {
            lessIndex = _lastLookupFileID._id;
        } else {
            maxIndex = _lastLookupFileID._id;
        }
    }

    unsigned tryNumber = 0;
    while (true) {
        --maxIndex;
        assert(maxIndex < _fileLocEntries.size());
        if (_fileLocEntries[maxIndex].getOffset() <= offset) {
            glu::FileID res(maxIndex);

            _lastLookupFileID = res;
            return res;
        }

        /// If the number of tries is greater than 8, we break the loop to try
        /// another type of search.
        if (++tryNumber == 8) {
            break;
        }
    }

    tryNumber = 0;

    while (true) {
        unsigned midIndex = (maxIndex - lessIndex) / 2 + lessIndex;
        std::size_t midOffset = _fileLocEntries[midIndex].getOffset();

        tryNumber++;

        if (midOffset > offset) {
            maxIndex = midIndex;
            continue;
        }

        if (midIndex + 1 == _fileLocEntries.size()
            || offset < _fileLocEntries[midIndex + 1].getOffset()) {
            glu::FileID res(midIndex);

            _lastLookupFileID = res;
            return res;
        }
        lessIndex = midIndex;
    }
}
