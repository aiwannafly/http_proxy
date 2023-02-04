#ifndef HTTP_CPP_PROXY_STATUS_CODE_H
#define HTTP_CPP_PROXY_STATUS_CODE_H

namespace status_code {
    static const int FAIL = -1;
    static const int SUCCESS = 0;
    static const int COMPLETED = 1;
    static const int TIMEOUT = 0;
    static const int TERMINATE  = -2;
}

#endif //HTTP_CPP_PROXY_STATUS_CODE_H
