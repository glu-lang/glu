#ifndef GLU_SOURCE_LOCATION_HPP
#define GLU_SOURCE_LOCATION_HPP

#include <cstdint>
#include <llvm/ADT/Hashing.h>

namespace glu {

class SourceManager;

///
/// @class FileID
/// @brief Contains an opaque identifier representing a file, used by the
/// SourceManager.
///
class FileID {
    friend class SourceManager;

    /// The opaque identifier for the file.
    int _id = 0;

private:
    ///
    /// @brief A FileID can only be created by the SourceManager (a friend
    /// class).
    ///
    /// @param id The opaque identifier for the file.
    /// @note This constructor is private to prevent clients from using it.
    ///
    FileID(int id) : _id(id) { }

public:
    FileID(FileID const &id) = default;
    FileID &operator=(FileID const &id) = default;
    FileID(FileID &&id) = default;
    FileID &operator=(FileID &&id) = default;

    bool operator==(FileID const &other) const { return _id == other._id; }
    bool operator!=(FileID const &other) const { return _id != other._id; }
};

///
/// @class SourceLocation
/// @brief Represents a specific location in the source code.
///
/// A SourceLocation is a lightweight object that represents a specific location
/// in the source code. It is used to refer to a specific character in a
/// specific file. The SourceManager class is responsible for creating and
/// interpreting SourceLocation objects.
///
/// A SourceLocation is basically an offset into the complete source code. The
/// SourceManager, knowing to which file this offset belongs, can interpret it
/// and provide useful information from it.
///
class SourceLocation {
    friend class SourceManager;

    /// The offset of the source location.
    uint32_t _offset = 0;

public:
    SourceLocation(uint32_t offset) : _offset(offset) { }

    static SourceLocation const invalid;

    bool operator==(SourceLocation const &other) const
    {
        return _offset == other._offset;
    }

    bool operator!=(SourceLocation const &other) const
    {
        return _offset != other._offset;
    }

    /// A SourceLocation can be invalid (ID == 0) in cases where there is no
    /// corresponding location in the source code. This typically occurs when
    /// diagnostics are generated for issues not tied to specific code, such as
    /// command-line option errors or internal compiler events. In such cases,
    /// the location cannot point to a valid file, line, or column in the code.
    bool isValid() const { return _offset != 0; }
    bool isInvalid() const { return _offset == 0; }

    /// @brief Get the offset of the source location.
    /// @return The offset of the source location.
    uint32_t getOffset() const { return _offset; }
};

inline glu::SourceLocation const glu::SourceLocation::invalid { 0 };

inline llvm::hash_code hash_value(glu::SourceLocation const &loc)
{
    return llvm::hash_value(loc.getOffset());
}

}

#endif // GLU_SOURCE_LOCATION_HPP
