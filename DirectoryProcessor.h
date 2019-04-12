#ifndef OS_FIND_DIRECTORYPROCESSOR_H
#define OS_FIND_DIRECTORYPROCESSOR_H

#include <string>

#include "Options.h"


class DirectoryProcessor {
    struct my_dirent64;
public:
    explicit DirectoryProcessor(Options options, char *env[]);

    void process(std::string path);

private:
    void processRootDirectory(std::string &path);

    void processEntry(std::string &path);

    void crawlDir(std::string const &absolutePath);

    bool checkDirent(my_dirent64 *dirent, std::string const &path);

    bool checkStat(std::string const &path);

    void execute(char *const *argv, char *const *envp);

    bool isDotOrDotDot(std::string const &s);

    struct my_dirent64 {
        ino64_t d_ino;    /* 64-bit inode number */
        off64_t d_off;    /* 64-bit offset to next structure */
        unsigned short d_reclen; /* Size of this dirent */
        unsigned char d_type;   /* File type */
        char d_name[]; /* Filename (null-terminated) */
    };

    char **env;
    Options options;

    const char * SEPARATOR = "/";
    const char CHAR_SEPARATOR = '/';
};


#endif //OS_FIND_DIRECTORYPROCESSOR_H