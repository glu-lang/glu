#ifndef GLU_SOURCE_LOCATION_HPP
#define GLU_SOURCE_LOCATION_HPP

#include <cstdint>

namespace glu {

class FileID {

public:
    bool operator==(FileID const &other) const
    {
        return id == other.id;
    }
    bool operator!=(FileID const &other) const
    {
        return id != other.id;
    }
    bool operator<(FileID const &other) const
    {
        return id < other.id;
    }
    bool operator>(FileID const &other) const
    {
        return id > other.id;
    }
    bool operator<=(FileID const &other) const
    {
        return id <= other.id;
    }
    bool operator>=(FileID const &other) const
    {
        return id >= other.id;
    }

private:
    friend class SourceManager;

    static FileID get(int id)
    {
        FileID fid;
        fid.id = id;
        return fid;
    }

private:
    int id = 0;
};

class SourceManager;

class SourceLocation {
    friend class SourceManager;

private:
    uint32_t _offset = 0;

public:
    SourceLocation(uint32_t offset)
        : _offset(offset)
    {
    }

    // A SourceLocation can be invalid (ID == 0) in cases where there is no
    // corresponding location in the source code. This typically occurs when
    // diagnostics are generated for issues not tied to specific code, such as
    // command-line option errors or internal compiler events. In such cases,
    // the location cannot point to a valid file, line, or column in the code.
    bool isValid() const
    {
        return _offset != 0;
    }
    bool isInvalid() const
    {
        return _offset == 0;
    }

private:
    // The offset is an opaque value that is used to refer to a specific
    // location in the source code. It is not intended to be used directly by
    // clients of the library, but is instead passed to other functions that
    // interpret it.
    uint32_t getOffset() const
    {
        return _offset;
    }
};

}
#endif // GLU_SOURCE_LOCATION_HPP
