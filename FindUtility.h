#ifndef OS_FIND_FINDUTILITY_H
#define OS_FIND_FINDUTILITY_H

#include <vector>
#include <string>
#include <optional>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <climits>
#include "Options.h"


class FindUtility {
public:
    void run(std::vector<std::string> const &args, char *env[]);

private:

    void printHelp();

    bool checkArgs(std::vector<std::string> const &tokens);

    bool updateOptions(std::string const &key, std::string const &value);

    template<typename T>
    bool setOptional(std::optional<T> &opt, T const &value);

    template<typename T>
    T getValueByType(std::string const &value);


    Options options;

};


#endif //OS_FIND_FINDUTILITY_H