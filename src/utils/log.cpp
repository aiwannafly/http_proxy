#include "log.h"

#include <iostream>

bool print_allowed = true;
std::string prefix = "[PROXY ] ";

void log(const std::string &msg) {
    if (!print_allowed) return;
    std::cout << prefix << msg << std::endl;
}

void logError(const std::string &msg) {
    if (!print_allowed) return;
    std::cerr << prefix << msg << std::endl;
}

void logErrorWithErrno(const std::string &msg) {
    if (!print_allowed) return;
    perror((prefix + msg).data());
}
