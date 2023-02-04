#ifndef HTTP_CPP_PROXY_IO_OPERATIONS_H
#define HTTP_CPP_PROXY_IO_OPERATIONS_H

#include <cstdlib>
#include <cstdio>
#include <string>

namespace io {
    static const int READ = 0;
    static const int WRITE = 1;
    static const int MSG_LENGTH_LIMIT = 32 * 1024;

    typedef struct Message {
    public:
        const char *data;
        size_t len;
        size_t capacity;

        Message() = default;

        ~Message() {
            free((void *) data);
        }

    } Message;

    Message *copy(Message *prev);

    bool AppendMsg(Message *a, Message *b);

    bool WriteAll(int fd, Message *message);

    Message *ReadAll(int socket_fd);
}

#endif //HTTP_CPP_PROXY_IO_OPERATIONS_H
