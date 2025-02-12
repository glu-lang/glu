#ifndef GLU_SOURCE_MANAGER_HPP
#define GLU_SOURCE_MANAGER_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <string>

#include "SourceLocation.hpp"

namespace glu {

///
/// @class FileLocEntry
/// @brief Represents a new file in the source code via an offset into the
/// complete source code.
///
/// The FileLocEntry class represents a new file in the source code. It is
/// responsible for storing the content of the file and providing information
/// about the file.
///
/// Also, an offset into the complete source code is stored. This offset is
/// unsed to quickly find a file that contains a specific SourceLocation. The
/// offset is incremented by the size of the file content.
///
class FileLocEntry {
    friend class SourceManager;

    /// This represent an offset into the complete source code.
    /// The offset is incremented by the size of the file content.
    ///
    /// For example, if the first file has a size of 100 characters, the _offset
    /// of the second file will be 100.
    /// If the second file has a size of 200 characters, the _offset of the
    /// third file will be 300.
    ///
    /// This offset is used to quickly find the file that contains a specific
    /// SourceLocation.
    uint32_t _offset;

    /// The location of the import directive that brought in the file.
    /// If the FileInfo is for the main file this location is invalid (ID == 0).
    SourceLocation _importLoc;

    std::string _fileName;

    /// The buffer containing the content of the file.
    std::unique_ptr<llvm::MemoryBuffer> _buffer;

public:
    FileLocEntry(
        uint32_t offset, std::unique_ptr<llvm::MemoryBuffer> buffer,
        SourceLocation importLoc, std::string fileName = ""
    )
        : _offset(offset)
        , _importLoc(importLoc)
        , _fileName(fileName)
        , _buffer(std::move(buffer))
    {
    }
    FileLocEntry(FileLocEntry &&other) = default;
    FileLocEntry &operator=(FileLocEntry &&other) = default;
    FileLocEntry(FileLocEntry const &other) = delete;
    FileLocEntry &operator=(FileLocEntry const &other) = delete;

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

    SourceLocation getImportLoc() const { return _importLoc; }

    /// Return the offset of the source location.
    uint32_t getOffset() const { return _offset; }

    llvm::StringRef getFileName() const { return _fileName; }
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

    /// Every source location of the source code.
    llvm::SmallVector<FileLocEntry, 0> _fileLocEntries;

    /// The starting offset of the next local FileLocEntry.
    ///
    /// This is _fileLocEntries.back()._offset + the size of that entry.
    std::size_t _nextOffset;

    /// TODO: This should be part of the FileManager class.
    /// The virtual file system used to load files.
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> _vfs;

    /// The FileID of the main file.
    FileID _mainFile;

public:
    SourceManager()
        : _nextOffset(0), _vfs(llvm::vfs::getRealFileSystem()), _mainFile(0)
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
    llvm::MemoryBuffer *getBuffer(FileID fileId) const;

    void setMainFileID(FileID fid) { _mainFile = fid; }
    FileID getMainFileID() const { return _mainFile; }

    FileID getFileID(uint32_t offset) const;
    bool isOffsetInFileID(FileID fid, unsigned offset) const;

    SourceLocation getLocForStartOfFile(FileID fileId) const;
    SourceLocation getLocForEndOfFile(FileID fileId) const;
    char const *getCharacterData(SourceLocation loc) const;
    SourceLocation getSourceLocFromStringRef(llvm::StringRef str) const;
    llvm::StringRef getBufferName(SourceLocation loc) const;

    unsigned getSpellingColumnNumber(SourceLocation loc) const;
    unsigned getSpellingLineNumber(SourceLocation loc) const;

    bool isInMainFile(SourceLocation loc) const
    {
        return getFileID(loc._offset) == _mainFile;
    }
    bool isWrittenInSameFile(SourceLocation loc1, SourceLocation loc2) const;
};

}

#endif // GLU_SOURCE_MANAGER_HPP
