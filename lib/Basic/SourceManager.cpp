#include "Basic/SourceManager.hpp"
#include "Basic/SourceLocation.hpp"

llvm::ErrorOr<glu::FileID>
glu::SourceManager::loadFile(llvm::StringRef filePath)
{
    llvm::SmallString<256> absPath(filePath);
    _vfs->makeAbsolute(absPath);
    llvm::sys::path::remove_filename(absPath);
    auto filename = llvm::sys::path::filename(filePath);

    // First check if the file is already loaded.
    for (unsigned i = 0; i < _fileLocEntries.size(); ++i) {
        if (_fileLocEntries[i]._absoluteDirName == absPath.str()
            && _fileLocEntries[i]._fileName == filename) {
            return glu::FileID(i);
        }
    }
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

    uint32_t fileOffset = _nextOffset;
    uint32_t fileSize = (*buffer)->getBufferSize();
    _nextOffset += fileSize;

    _fileLocEntries.emplace_back(
        fileOffset, std::move(*buffer), SourceLocation(fileOffset),
        filename.str(), absPath.str().str()
    );

    if (_fileLocEntries.size() == 1) {
        _mainFile = FileID(0);
    }

    return llvm::ErrorOr<glu::FileID>(glu::FileID(_fileLocEntries.size() - 1));
}

llvm::MemoryBuffer *glu::SourceManager::getBuffer(FileID fileId) const
{
    if (fileId._id >= static_cast<int>(_fileLocEntries.size())) {
        return nullptr;
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::MemoryBufferRef> bufferOpt = entry.getBufferIfLoaded();

    if (!bufferOpt) {
        return nullptr;
    }

    return entry._buffer.get();
}

glu::FileID glu::SourceManager::getFileID(SourceLocation loc) const
{
    for (unsigned i = 0; i < _fileLocEntries.size(); ++i) {
        if (isOffsetInFileID(FileID(i), loc._offset)) {
            return FileID(i);
        }
    }

    return FileID(-1);
}

bool glu::SourceManager::isOffsetInFileID(
    glu::FileID fid, SourceLocation loc
) const
{
    if (fid._id >= static_cast<int>(_fileLocEntries.size())) {
        return false;
    }

    auto const &entry = _fileLocEntries[fid._id];
    uint32_t fileStart = entry.getOffset();
    uint32_t fileEnd = fileStart + entry.getSize();

    return (loc._offset >= fileStart && loc._offset < fileEnd);
}

glu::SourceLocation
glu::SourceManager::getLocForStartOfFile(FileID fileID) const
{
    if (fileID._id >= static_cast<int>(_fileLocEntries.size())) {
        return SourceLocation(-1);
    }

    auto const &entry = _fileLocEntries[fileID._id];
    uint32_t startOffset = entry.getOffset();
    return SourceLocation(startOffset);
}

glu::SourceLocation
glu::SourceManager::getSourceLocFromStringRef(llvm::StringRef str) const
{
    for (auto const &entry : _fileLocEntries) {
        std::optional<llvm::StringRef> bufferOpt
            = entry.getBufferDataIfLoaded();
        if (!bufferOpt)
            continue;

        llvm::StringRef buffer = *bufferOpt;
        if (str.data() >= buffer.data()
            && str.data() < buffer.data() + buffer.size()) {
            return SourceLocation(
                entry.getOffset() + (str.data() - buffer.data())
            );
        }
    }
    return SourceLocation(0);
}

glu::SourceLocation
glu::SourceManager::getSourceLocFromToken(glu::Token tok) const
{
    return getSourceLocFromStringRef(tok.getLexeme());
}

char const *glu::SourceManager::getCharacterData(SourceLocation loc) const
{
    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return nullptr;
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::StringRef> bufferOpt = entry.getBufferDataIfLoaded();

    if (!bufferOpt) {
        return nullptr;
    }

    llvm::StringRef buffer = *bufferOpt;
    unsigned offsetInFile = loc._offset - entry.getOffset();

    if (offsetInFile >= buffer.size()) {
        return nullptr;
    }

    return buffer.data() + offsetInFile;
}

unsigned glu::SourceManager::getSpellingColumnNumber(SourceLocation loc) const
{
    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return 0;
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::StringRef> bufferOpt = entry.getBufferDataIfLoaded();

    if (!bufferOpt) {
        return 0;
    }
    llvm::StringRef buffer = *bufferOpt;

    unsigned offsetInFile = loc._offset - entry.getOffset();
    unsigned column = 1;

    for (unsigned i = offsetInFile; i > 0; --i) {
        if (buffer[i - 1] == '\n') {
            break;
        }
        column++;
    }

    return column;
}

unsigned glu::SourceManager::getSpellingLineNumber(SourceLocation loc) const
{
    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return 0;
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::StringRef> bufferOpt = entry.getBufferDataIfLoaded();

    if (!bufferOpt) {
        return 0;
    }
    llvm::StringRef buffer = *bufferOpt;

    unsigned offsetInFile = loc._offset - entry.getOffset();
    unsigned line = 1;

    for (unsigned i = 0; i < offsetInFile; ++i) {
        if (buffer[i] == '\n') {
            line++;
        }
    }

    return line;
}

void glu::SourceManager::loadBuffer(
    std::unique_ptr<llvm::MemoryBuffer> buffer, std::string fileName
)
{
    uint32_t fileOffset = _nextOffset;
    uint32_t fileSize = buffer->getBufferSize();
    _nextOffset += fileSize;

    _fileLocEntries.emplace_back(
        fileOffset, std::move(buffer), SourceLocation(fileOffset), fileName
    );
}

llvm::StringRef glu::SourceManager::getBufferName(SourceLocation loc) const
{
    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return "<unknown file>";
    }

    auto const &entry = _fileLocEntries[fileId._id];
    return entry.getFileName();
}

void glu::SourceManager::reset()
{
    _fileLocEntries.clear();
    _nextOffset = 0;
    _mainFile = FileID(0);
}

glu::SourceLocation glu::SourceManager::getLineStart(SourceLocation loc) const
{
    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return SourceLocation::invalid;
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::StringRef> bufferOpt = entry.getBufferDataIfLoaded();

    if (!bufferOpt) {
        return SourceLocation::invalid;
    }

    llvm::StringRef buffer = *bufferOpt;
    unsigned offsetInFile = loc._offset - entry.getOffset();

    // Find start of the line containing the location
    char const *bufStart = buffer.data();
    char const *pos = bufStart + offsetInFile;

    while (pos > bufStart && pos[-1] != '\n') {
        --pos;
    }

    return SourceLocation((pos - bufStart) + entry.getOffset());
}

glu::SourceLocation glu::SourceManager::getLineEnd(SourceLocation loc) const
{
    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return SourceLocation::invalid;
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::StringRef> bufferOpt = entry.getBufferDataIfLoaded();

    if (!bufferOpt) {
        return SourceLocation::invalid;
    }

    llvm::StringRef buffer = *bufferOpt;
    unsigned offsetInFile = loc._offset - entry.getOffset();

    // Find end of the line containing the location
    char const *bufStart = buffer.data();
    char const *bufEnd = bufStart + buffer.size();
    char const *pos = bufStart + offsetInFile;

    while (pos < bufEnd && *pos != '\n' && *pos != '\r') {
        ++pos;
    }

    return SourceLocation((pos - bufStart) + entry.getOffset());
}

llvm::StringRef glu::SourceManager::getLine(SourceLocation loc) const
{
    auto start = getLineStart(loc);
    auto end = getLineEnd(loc);

    if (start.isInvalid() || end.isInvalid()) {
        return "";
    }

    FileID fileId = getFileID(loc);
    if (fileId._id == -1) {
        return "";
    }

    auto const &entry = _fileLocEntries[fileId._id];
    std::optional<llvm::StringRef> bufferOpt = entry.getBufferDataIfLoaded();

    if (!bufferOpt) {
        return "";
    }

    llvm::StringRef buffer = *bufferOpt;
    auto startOffset = start.getOffset();
    auto endOffset = end.getOffset();

    char const *bufStart = buffer.data();
    char const *startPos = bufStart + (startOffset - entry.getOffset());
    char const *endPos = bufStart + (endOffset - entry.getOffset());
    return llvm::StringRef(startPos, endPos - startPos);
}
