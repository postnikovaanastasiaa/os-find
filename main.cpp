#include <iostream>
#include "FindUtility.h"

// I don't want to use argc later :(
std::vector<std::string> getVector(int argc, char * argv[]) {
    std::vector<std::string> args;
    args.reserve(argc - 1);
    for (int  i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    return args;
}

int main(int argc, char *argv[], char * env[]) {
    FindUtility findUtility;
    findUtility.run(getVector(argc, argv), env);
}