#ifndef GLU_SOURCE_MANAGER_HPP
#define GLU_SOURCE_MANAGER_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/VirtualFileSystem.h>

#include "SourceLocation.hpp"

namespace glu {

///
/// @class ContentCache
/// @brief A file loaded will be cached in this class.
///
/// The ContentCache class is responsible for caching the content of a file that
/// has been loaded. It is used by the SourceManager to store the content of a
/// file. The content of a file is stored in a MemoryBuffer object which is
/// owned by the ContentCache.
///
class alignas(8) ContentCache {

private:
    llvm::StringRef _fileName;

    /// The buffer containing the content of the file.
    mutable std::unique_ptr<llvm::MemoryBuffer> _buffer;
    /// A flag to indicate if the buffer is invalid.
    mutable unsigned _isBufferInvalid : 1;

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
};

///
/// @class FileInfo
/// @brief Contains concrete information about a file.
///
/// The FileInfo class contains information about a file that has been loaded by
/// the SourceManager. It contains the SourceLocation of the import directive
/// that brought in the file, and a reference to the ContentCache object that
/// contains the content of the file.
///
class FileInfo {

private:
    /// The location of the import directive that brought in the file.
    /// If the FileInfo is for the main file this location is invalid (ID == 0).
    SourceLocation _importLoc;

    /// The content cache of the file.
    ContentCache const *_contentCache;

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
};

///
/// @class SourceLocEntry
/// @brief Represents a specific location in the source code.
///
/// A SourceLocEntry is an object that represents a specific location
/// in the source code. It is used to refer to a specific character in a
/// specific file. The SourceManager class is responsible for creating and
/// interpreting SourceLocEntry objects.
///
/// A SourceLocEntry is basicly an offset into the complete source code. The
/// SourceManager knowings to which file this offset belongs can interpret it
/// and provide useful informations from it.
///
class SourceLocEntry {

private:
    /// The offset of the source location.
    uint32_t _offset;

    /// The file information for the source location.
    FileInfo _file;

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
};

///
/// @class SourceManager
/// @brief Manages the source code of a glu program.
///
/// The SourceManager class is responsible for managing the source code of a glu
/// program. For now, it is responsible for loading files, caching their
/// content, and for providing information about the source code. At term, the
/// FileManager will be responsible of the file system and the SourceManager
/// will be responsible for the source code.
///
/// The SourceManager knows how to interpret a SourceLocation object and provide
/// information about the source code at that location in an efficient way.
///
class SourceManager {

private:
    /// Every source location of the source code.
    llvm::SmallVector<SourceLocEntry, 0> _sourceLocs;
    /// The content caches of the loaded files.
    std::vector<ContentCache *> _contentCaches;

    /// TODO: This should be part of the FileManager class.
    /// The virtual file system used to load files.
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> _vfs;

    /// The FileID of the main file.
    FileID _mainFile;

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

    ///
    /// TODO: This function must be part of the FileManager class.
    ///
    /// Load a file from the file system. The file is loaded using the virtual
    /// file system. The content of the file is stored in a ContentCache object
    /// and the file is assigned a FileID.
    ///
    /// @param filePath The path to the file to load.
    /// @return A FileID object that represents the file that has been loaded.
    ///
    llvm::ErrorOr<FileID> loadFile(llvm::StringRef filePath);

    /// Set the file ID for the main source file.
    void setMainFileID(FileID fid)
    {
        _mainFile = fid;
    }

    FileID getMainFileID() const
    {
        return _mainFile;
    }

    /// TODO: Those functions needs to be implemented.
    SourceLocation getLocForStartOfFile(FileID fileId) const;
    SourceLocation getLocForEndOfFile(FileID fileId) const;

    /// TODO: This function needs to be implemented.
    SourceLocation getSpellingLoc(SourceLocation loc) const;

    /// TODO: This function needs to be implemented.
    char const *getCharacterData(SourceLocation loc) const;

    /// TODO: Those functions needs to be implemented.
    unsigned getSpellingColumnNumber(SourceLocation loc) const;
    unsigned getSpellingLineNumber(SourceLocation loc) const;

    /// TODO: This function needs to be implemented.
    llvm::StringRef getBufferName(SourceLocation loc) const;

    /// TODO: Those functions needs to be implemented.
    bool isInMainFile(SourceLocation loc) const;
    bool isWrittenInSameFile(SourceLocation loc1, SourceLocation loc2) const;
};

}

#endif // GLU_SOURCE_MANAGER_HPP
