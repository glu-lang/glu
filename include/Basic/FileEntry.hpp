#ifndef GLU_FILE_ENTRY_HPP
#define GLU_FILE_ENTRY_HPP

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <ctime>
#include <memory>

namespace glu {

class FileEntry {
    friend class FileManager;

    FileEntry() = default;
    FileEntry(FileEntry const &other) = delete;
    FileEntry &operator=(FileEntry const &other) = delete;

public:
    ~FileEntry();
    llvm::StringRef getName() const
    {
        return _filePath;
    }
    unsigned int getSize() const
    {
        return _fileSize;
    }
    time_t getModificationTime() const
    {
        return _modificationTime;
    }

    bool isNamedPipe() const
    {
        return _isNamedPipe;
    }

private:
    llvm::StringRef _filePath;
    unsigned int _fileSize = 0;
    time_t _modificationTime = 0;
    bool _isNamedPipe = false;

    mutable std::unique_ptr<llvm::vfs::File> File;

    bool _isBufferInvalid = true;
};

class FileManager {

public:
    FileManager()
    {
        _vfs = llvm::vfs::getRealFileSystem();
    }
    FileManager(FileManager const &other) = delete;
    FileManager &operator=(FileManager const &other) = delete;

    ~FileManager();

    llvm::ErrorOr<FileEntry const *> getFile(llvm::StringRef filePath)
    {
        llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> file
            = _vfs->openFileForRead(filePath);
        llvm::vfs::Status status;

        if (!file) {
            return file.getError();
        }
        llvm::ErrorOr<llvm::vfs::Status> statusOrErr = (*file)->status();

        if (!statusOrErr) {
            return statusOrErr.getError();
        }

        status = *statusOrErr;
        FileEntry *entry = new FileEntry();

        entry->_fileSize = status.getSize();
        entry->_modificationTime
            = llvm::sys::toTimeT(status.getLastModificationTime());
        entry->_isNamedPipe
            = status.getType() == llvm::sys::fs::file_type::fifo_file;
        entry->File = std::move(*file);
        entry->_filePath = filePath;

        return entry;
    }

private:
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> _vfs;
};

}

#endif // GLU_FILE_ENTRY_HPP
