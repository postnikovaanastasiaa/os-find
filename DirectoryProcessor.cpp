#include <utility>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <syscall.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <wait.h>

#include <iostream>
#include <utility>
#include <sstream>
#include <memory>

#include "DirectoryProcessor.h"

DirectoryProcessor::DirectoryProcessor(Options options, char *env[]) : options(std::move(options)), env(env) {}

void DirectoryProcessor::process(std::string path) {
    if (path.back() != CHAR_SEPARATOR) {
        path += SEPARATOR;
    }
    processRootDirectory(path);
    crawlDir(path);
}

void DirectoryProcessor::processRootDirectory(std::string &path) {
    if (options.name.has_value()) {
        auto slash = path.rfind(CHAR_SEPARATOR, path.length() < 2 ? path.length() : path.length() - 2);
        std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1,
                path.back() == CHAR_SEPARATOR? path.length() - slash - 2 : std::string::npos);
        if (options.name.value() != name) {
            return;
        }
    }
    if (checkStat(path)) {
        processEntry(path);
    }
}

void DirectoryProcessor::processEntry(std::string &path) {
    if (options.exec.has_value()) {
        std::unique_ptr<char *[]> args(new char *[3]);

        args[0] = options.exec.value().data();
        args[1] = path.data();
        args[2] = nullptr;
        execute(args.get(), env);
    } else {
        std::cout << path << std::endl;
    }
}

void DirectoryProcessor::crawlDir(std::string const &absolutePath) {
    int fd = open(absolutePath.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror(("Directory by path" + absolutePath + "was not opened").c_str());
        return;
    }
    std::vector<std::string> dirs;
    int bytesNum;
    static size_t BUF_SIZE = 4096;
    char buffer[BUF_SIZE];

    while ((bytesNum = syscall(SYS_getdents64, fd, buffer, BUF_SIZE)) != 0) {
        if (bytesNum == -1) {
            perror("Failed while getting dir entries");
            return;
        }
        my_dirent64 *d;
        size_t pos = 0;
        while (pos < bytesNum) {
            d = (my_dirent64 *) (buffer + pos);
            pos += d->d_reclen;
            std::string direntPath = absolutePath + d->d_name;
            if (isDotOrDotDot(d->d_name)) {
                continue;
            }
            if (d->d_type == DT_DIR) {
                dirs.push_back(std::string(d->d_name) + SEPARATOR);
            }
            if (checkDirent(d, direntPath)) {
                processEntry(direntPath);
            }
        }
    }
    if (close(fd) == -1) {
        perror("Error occurred while closing a file descriptor");
    }
    for (auto &dir : dirs) {
        crawlDir(absolutePath + dir);
    }
}

bool DirectoryProcessor::checkDirent(DirectoryProcessor::my_dirent64 *dirent, std::string const &path) {
    bool okValue = !options.name.has_value() || options.name.value() == dirent->d_name;
    bool okInode = !options.inode.has_value() || options.inode.value() == dirent->d_ino;

    if (!options.hardlinks.has_value() && options.minSize == 0 && options.maxSize == Options::MAX_FILE_SIZE) {
        // don't need stat
        return okValue && okInode;
    }
    if (!okInode || !okValue) {
        return false;
    }
    return checkStat(path);
}

bool DirectoryProcessor::checkStat(std::string const &path) {
    struct stat statBuf;
    int err = stat(path.c_str(), &statBuf);

    if (err == -1) {
        perror("Could not access stat");
        return false;
    }
    bool okHardlinks = !options.hardlinks.has_value() || options.hardlinks.value() == statBuf.st_nlink;
    bool okSize = options.minSize <= statBuf.st_size && options.maxSize >= statBuf.st_size;
    bool okInode = !options.inode.has_value() || options.inode.value() == statBuf.st_ino;

    return okHardlinks && okInode && okSize;
}


void DirectoryProcessor::execute(char *const *argv, char *const *envp) {
    const pid_t pid = fork();

    if (pid == -1) {
        perror("Could not create child process");

    } else if (pid == 0) {
        const int err = execve(argv[0], argv, envp);

        if (err == -1) {
            perror("Execution failed");
            exit(-1);
        }
    } else {
        int status;
        const pid_t id = waitpid(pid, &status, 0);
        if (id == -1) {
            perror("Execution failed");
        }
    }
}

bool DirectoryProcessor::isDotOrDotDot(std::string const &s) {
    return s == "." || s == "..";
}