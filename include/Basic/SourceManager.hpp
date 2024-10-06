#ifndef GLU_SOURCE_MANAGER_HPP
#define GLU_SOURCE_MANAGER_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/MemoryBuffer.h>
#include <optional>

#include "FileEntry.hpp"
#include "SourceLocation.hpp"
#include "llvm/ADT/StringRef.h"

namespace glu::srcmanager {

/// This object owns the MemoryBuffer object containing a file contents.
class alignas(8) ContentCache {

public:
    std::optional<FileEntry> _fileEntry;

public:
    ContentCache(llvm::StringRef fileName)
        : _fileEntry(std::nullopt)
        , _fileName(fileName)
        , _isBufferInvalid(true)
    {
    }
    ContentCache(ContentCache const &other) = delete;
    ContentCache &operator=(ContentCache const &other) = delete;

    unsigned getSize() const
    {
        return _buffer ? (unsigned) _buffer->getBufferSize() : 0;
    }

    /// Return the buffer, only if it has been loaded.
    std::optional<llvm::MemoryBufferRef> getBufferIfLoaded() const
    {
        if (_buffer)
            return _buffer->getMemBufferRef();
        return std::nullopt;
    }

    /// Return a StringRef to the source buffer data, only if it has already
    /// been loaded.
    std::optional<llvm::StringRef> getBufferDataIfLoaded() const
    {
        if (_buffer)
            return _buffer->getBuffer();
        return std::nullopt;
    }

    std::optional<llvm::MemoryBufferRef> getBufferOrNone(FileManager &fm);

    /// Set the MemoryBuffer for this ContentCache.
    void setBuffer(std::unique_ptr<llvm::MemoryBuffer> buffer)
    {
        _isBufferInvalid = false;
        _buffer = std::move(buffer);
    }

    void setFileName(llvm::StringRef fileName)
    {
        _fileName = fileName;
    }

    llvm::StringRef getFileName() const
    {
        return _fileName;
    }

private:
    llvm::StringRef _fileName;

    mutable std::unique_ptr<llvm::MemoryBuffer> _buffer;
    mutable unsigned _isBufferInvalid : 1;
};

class FileInfo {

public:
    FileInfo()
        : _includeLoc(0)
        , _contentCache(nullptr) {};

    static FileInfo
    get(SourceLocation includeLoc, ContentCache &contentCache,
        llvm::StringRef fileName)
    {
        FileInfo info;

        info._includeLoc = includeLoc;
        info._contentCache = &contentCache;
        contentCache.setFileName(fileName);
        return info;
    }

    SourceLocation getIncludeLoc() const
    {
        return _includeLoc;
    }

    ContentCache const &getContentCache() const
    {
        return *_contentCache;
    }

    llvm::StringRef getFileName() const
    {
        return _contentCache->getFileName();
    }

private:
    SourceLocation _includeLoc;

    /// The content cache and the characteristic of the file.
    ContentCache const *_contentCache;
};

class SourceLocEntry {

public:
    SourceLocEntry()
        : _offset()
        , _isExpansion()
    {
    }

    FileInfo const &getFileInfo() const
    {
        return const_cast<SourceLocEntry *>(this)->getFile();
    }
    FileInfo &getFile()
    {
        return _file;
    }

    uint32_t getOffset() const
    {
        return _offset;
    }

    bool isExpansion() const
    {
        return _isExpansion;
    }

    static SourceLocEntry get(uint32_t offset, FileInfo const &file)
    {
        SourceLocEntry entry;
        entry._offset = offset;
        entry._isExpansion = false;
        entry._file = file;
        return entry;
    }

private:
    uint32_t _offset;
    uint32_t _isExpansion : 1;

    FileInfo _file;
};

class SourceManager {

public:
    SourceManager();
    SourceManager(SourceManager const &other) = delete;
    SourceManager &operator=(SourceManager const &other) = delete;
    SourceManager(SourceManager &&other) = delete;
    SourceManager &operator=(SourceManager &&other) = delete;

    ~SourceManager() = default;

private:
    llvm::SmallVector<SourceLocEntry, 0> _sourceLocs;
    std::vector<ContentCache *> MemBufferInfos;

    FileID _mainFile;

public:
    /// Set the file ID for the main source file.
    void setMainFileID(FileID fid)
    {
        _mainFile = fid;
    }

    FileID getMainFileID() const
    {
        return _mainFile;
    }
};

}

#endif // GLU_SOURCE_MANAGER_HPP
