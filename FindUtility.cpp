#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <error.h>
#include <syscall.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <wait.h>
#include <errno.h>

#include <iostream>
#include <memory>
#include <utility>
#include <sstream>

#include "FindUtility.h"
#include "DirectoryProcessor.h"


void FindUtility::run(std::vector<std::string> const &args, char *env[]) {
    if (args.empty()) {
        std::cout << "Absolute path must be the first argument.\n"
                     "Print \"--help\" to learn more." << std::endl;
        return;
    }
    if (args[0] == "--help") {
        printHelp();
    } else {
        if (checkArgs(args)) {
            DirectoryProcessor processor(options, env);
            processor.process(args[0]);
        } else {
            std::cout << "Wrong arguments format.\n"
                         "Print \"--help\" to learn more." << std::endl;
        }
    }
}


bool FindUtility::checkArgs(std::vector<std::string> const &tokens) {
    if (tokens.size() % 2 == 0) {
        return false;
    }
    for (size_t i = 1; i < tokens.size(); i += 2) {
        auto &key = tokens[i];
        auto &value = tokens[i + 1];
        if (!updateOptions(key, value)) {
            return false;
        }
    }
    return true;
}

template<typename T>
T FindUtility::getValueByType(std::string const &value) {
    std::istringstream stream(value);
    T res;
    stream >> res;
    return res;
}

bool FindUtility::updateOptions(std::string const &key, std::string const &value) {
    if (key == "-size") {
        auto size = getValueByType<Options::file_size_t>(value.substr(1));
        switch (value[0]) {
            case '+':
                if (size > options.minSize) {
                    options.minSize = size;
                }
                break;
            case '-':
                if (size < options.maxSize) {
                    options.maxSize = size;
                }
                break;
            case '=':
                if (size >= options.minSize && size <= options.maxSize) {
                    options.maxSize = options.minSize = size;
                    break;
                } else {
                    return false;
                }
            default:
                return false;
        }
    } else if (key == "-name") {
        return setOptional(options.name, value);
    } else if (key == "-exec") {
        return setOptional(options.exec, value);
    } else if (key == "-nlinks") {
        return setOptional(options.hardlinks, getValueByType<nlink_t>(value));
    } else if (key == "-inum") {
        return setOptional(options.inode, getValueByType<ino64_t>(value));
    }
    return options.minSize <= options.maxSize;
}

void FindUtility::printHelp() {
    std::cout << "find --help\n"
                 "Usage: find PATH [flag ...]\n"
                 "\n"
                 "PATH must be absolute path of the directory to run find in.\n"
                 "Utility crawls the provided directory recursively.\n"
                 "Prints visited files and directories by default.\n"
                 "Flags filter visited dirs and files.\n"
                 "\n"
                 "Flags:"
                 "-size [[-+=]SIZE]     files with size less(or equal), more(or equal) or equal to SIZE\n"
                 "-name NAME            files with the simple name equals to NAME\n"
                 "-inum NUM             files with inode number NUM\n"
                 "-nlinks NUM           files with hardlinks number NUM\n"
                 "-exec EXEC            execute absolute path EXEC with PATH as an argument\n"
                 "\n"
                 "A few -size flags are supported. The result is intersection of the arguments.\n"
                 "Other flags can be used only once.\n" << std::endl;
}

template<typename T>
bool FindUtility::setOptional(std::optional<T> &opt, T const &value) {
    if (opt.has_value()) {
        return false;
    }
    opt = value;
    return true;
}