#ifndef OS_FIND_OPTIONS_H
#define OS_FIND_OPTIONS_H

#include <vector>
#include <string>
#include <optional>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <climits>

struct Options {
    using file_size_t = uint64_t;
    static const file_size_t MAX_FILE_SIZE = UINT64_MAX;

    file_size_t minSize;
    file_size_t maxSize;

    std::optional<std::string> name;
    std::optional<ino64_t> inode;
    std::optional<std::string> exec;
    std::optional<nlink_t> hardlinks;

    Options() : minSize(0), maxSize(MAX_FILE_SIZE) {}
};

#endif //OS_FIND_OPTIONS_H
