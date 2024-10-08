#ifndef GLU_SOURCE_MANAGER_HPP
#define GLU_SOURCE_MANAGER_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/MemoryBuffer.h>
#include <optional>

#include "SourceLocation.hpp"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace glu {

/**
 * @class ContentCache
 * @brief A file loaded will be cached in this class.
 *
 * The ContentCache class is responsible for caching the content of a file that
 * has been loaded. It is used by the SourceManager to store the content of a
 * file. The content of a file is stored in a MemoryBuffer object which is owned
 * by the ContentCache.
 *
 */
class alignas(8) ContentCache {

public:
    ContentCache(llvm::StringRef fileName)
        : _fileName(fileName)
        , _isBufferInvalid(true)
    {
    }
    /// Copy constructor and assignment operator are deleted.
    /// A unique owned file content should not be copied.
    ContentCache(ContentCache const &other) = delete;
    ContentCache &operator=(ContentCache const &other) = delete;

    /// Get the size of the loaded buffer if loaded.
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

    /// The buffer containing the content of the file.
    mutable std::unique_ptr<llvm::MemoryBuffer> _buffer;
    /// A flag to indicate if the buffer is invalid.
    mutable unsigned _isBufferInvalid : 1;
};

/**
 * @class FileInfo
 * @brief Contains concrete information about a file.
 *
 * The FileInfo class contains information about a file that has been loaded by
 * the SourceManager. It contains the SourceLocation of the import directive
 * that brought in the file, and a reference to the ContentCache object that
 * contains the content of the file.
 *
 */
class FileInfo {

public:
    FileInfo()
        : _importLoc(0)
        , _contentCache(nullptr) {};

    /// Create a FileInfo object with the given SourceLocation and ContentCache.
    static FileInfo
    get(SourceLocation includeLoc, ContentCache &contentCache,
        llvm::StringRef fileName)
    {
        FileInfo info;

        info._importLoc = includeLoc;
        info._contentCache = &contentCache;
        contentCache.setFileName(fileName);
        return info;
    }

    SourceLocation getImportLoc() const
    {
        return _importLoc;
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
    /// The location of the import directive that brought in the file.
    /// If the FileInfo is for the main file this location is invalid (ID == 0).
    SourceLocation _importLoc;

    /// The content cache of the file.
    ContentCache const *_contentCache;
};

/**
 * @class SourceLocEntry
 * @brief Represents a specific location in the source code.
 *
 * A SourceLocEntry is an object that represents a specific location
 * in the source code. It is used to refer to a specific character in a specific
 * file. The SourceManager class is responsible for creating and interpreting
 * SourceLocEntry objects.
 *
 * A SourceLocEntry is basicly an offset into the complete source code. The
 * SourceManager knowings to which file this offset belongs can interpret it and
 * provide useful informations from it.
 */
class SourceLocEntry {

public:
    SourceLocEntry(uint32_t offset, FileInfo const &file)
        : _offset(offset)
        , _file(file)
    {
    }

    /// Returnn a const reference to the FileInfo object.
    FileInfo const &getFileInfo() const
    {
        return const_cast<SourceLocEntry *>(this)->getFile();
    }

    FileInfo &getFile()
    {
        return _file;
    }

    /// Return the offset of the source location.
    uint32_t getOffset() const
    {
        return _offset;
    }

private:
    /// The offset of the source location.
    uint32_t _offset;

    /// The file information for the source location.
    FileInfo _file;
};

class SourceManager {
public:
    SourceManager()
        : _vfs(llvm::vfs::getRealFileSystem())
        , _mainFile(0)
    {
    }
    SourceManager(SourceManager const &other) = delete;
    SourceManager &operator=(SourceManager const &other) = delete;
    SourceManager(SourceManager &&other) = delete;
    SourceManager &operator=(SourceManager &&other) = delete;

    ~SourceManager() = default;

    llvm::ErrorOr<FileID> loadFile(llvm::StringRef filePath)
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

        _contentCaches.push_back(cache);

        auto fileName = (*file)->getName();

        if (!fileName) {
            return fileName.getError();
        }

        SourceLocEntry entry(
            _sourceLocs.size(),
            FileInfo::get(SourceLocation(0), *cache, *fileName)
        );

        if (_sourceLocs.empty()) {
            _mainFile = FileID(_sourceLocs.size());
        }

        return FileID(_sourceLocs.size());
    }

    /// Set the file ID for the main source file.
    void setMainFileID(FileID fid)
    {
        _mainFile = fid;
    }

    FileID getMainFileID() const
    {
        return _mainFile;
    }

private:
    llvm::SmallVector<SourceLocEntry, 0> _sourceLocs;
    std::vector<ContentCache *> _contentCaches;

    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> _vfs;
    FileID _mainFile;
};

}

#endif // GLU_SOURCE_MANAGER_HPP
