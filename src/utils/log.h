#ifndef PTHREAD_HTTP_PROXY_LOG_H
#define PTHREAD_HTTP_PROXY_LOG_H

#include <string>

void log(const std::string &msg);

void logError(const std::string &msg);

void logErrorWithErrno(const std::string &msg);

#endif //PTHREAD_HTTP_PROXY_LOG_H
