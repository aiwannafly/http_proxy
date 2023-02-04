#include "io_operations.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>

#include "../status_code.h"

namespace io {
    static const int DEFAULT_BUFFER_SIZE = 128;

    Message *copy(Message *prev) {
        assert(prev);
        auto *res = new Message;
        res->len = prev->len;
        res->capacity = prev->capacity;
        res->data = (char *) malloc(res->capacity);
        memcpy((void *) res->data, prev->data, res->len);
        return res;
    }

    bool AppendMsg(Message *a, Message *b) {
        assert(a);
        assert(b);
        size_t required_length = a->len + b->len + 1;
        while (a->capacity < required_length) {
            a->capacity *= 2;
            char *temp = (char *) realloc((void *) a->data, a->capacity);
            if (temp == nullptr) {
                return false;
            }
            a->data = temp;
        }
        memcpy((void *) (a->data + a->len), b->data, b->len);
        a->len += b->len;
        return true;
    }

    bool WriteAll(int fd, Message *message) {
        if (nullptr == message) {
            return false;
        }
        if (nullptr == message->data) {
            return false;
        }
        size_t written_bytes = 0;
        size_t left_to_write = message->len;
        while (true) {
            ssize_t count = write(fd, message->data + written_bytes, left_to_write);
            if (count == status_code::FAIL) {
                return false;
            }
            written_bytes += count;
            if (written_bytes == message->len) {
                return true;
            }
            left_to_write -= count;
        }
    }

    bool fwrite_into_pipe(FILE *pipe_fd, char *buffer, size_t len) {
        if (nullptr == buffer || nullptr == pipe_fd) {
            return false;
        }
        size_t written_bytes = 0;
        size_t left_to_write = len;
        while (true) {
            size_t count = fwrite(buffer + written_bytes, sizeof(char),  left_to_write, pipe_fd);
            if (ferror(pipe_fd)) {
                return false;
            }
            written_bytes += count;
            if (written_bytes == len) {
                return true;
            }
            left_to_write -= count;
        }
    }

    Message *ReadAll(int socket_fd) {
        size_t capacity = DEFAULT_BUFFER_SIZE;
        char *buffer = (char *) malloc(capacity + 1);
        if (buffer == nullptr) {
            return nullptr;
        }
        size_t offset = 0;
        size_t portion = DEFAULT_BUFFER_SIZE;
        while (true) {
            if (offset + portion > capacity) {
                capacity *= 2;
                char *temp = (char *) realloc(buffer, capacity);
                if (nullptr == temp) {
                    free(buffer);
                    return nullptr;
                }
                buffer = temp;
            }
            long read_bytes = read(socket_fd, buffer + offset, portion);
            if (status_code::FAIL == read_bytes) {
                if (errno == EINTR) {
                    continue;
                } else if (errno == EAGAIN) {
                    buffer[offset] = '\0';
                    auto *message = new Message;
                    message->data = buffer;
                    message->len = offset;
                    message->capacity = capacity;
                    return message;
                } else {
                    free(buffer);
                    return nullptr;
                }
            }
            if (0 == read_bytes || offset >= MSG_LENGTH_LIMIT) {
                offset += read_bytes;
                break;
            } else {
                offset += read_bytes;
                if (offset % portion != 0) {
                    break;
                }
            }
        }
        buffer[offset] = '\0';
        auto *message = new Message;
        message->data = buffer;
        message->len = offset;
        message->capacity = capacity;
        return message;
    }

    char *read_from_file(int pipe_fd) {
        size_t capacity = DEFAULT_BUFFER_SIZE;
        char *buffer = (char *) malloc(capacity);
        size_t offset = 0;
        size_t portion = DEFAULT_BUFFER_SIZE;
        while (true) {
            printf("offset: %zu\n", offset);
            if (offset + portion > capacity) {
                capacity *= 2;
                char *temp = (char *) realloc(buffer, capacity);
                if (nullptr == temp) {
                    free(buffer);
                    return nullptr;
                }
                buffer = temp;
            }
            if (buffer[offset] == 0) {
                break;
            }
            long read_bytes = read(pipe_fd, buffer + offset, portion);
            if (status_code::FAIL == read_bytes) {
                if (errno == EINTR) {
                    continue;
                } else {
                    free(buffer);
                    return nullptr;
                }
            }
            if (0 == read_bytes) {
                break;
            } else {
                offset += read_bytes;
            }
        }
        buffer[offset] = '\0';
        return buffer;
    }

    char *fread_from_pipe(FILE *pipe_fp) {
        size_t capacity = DEFAULT_BUFFER_SIZE;
        char *buffer = (char *) malloc(capacity);
        size_t offset = 0;
        size_t portion = DEFAULT_BUFFER_SIZE;
        while (true) {
            if (offset + portion > capacity) {
                capacity *= 2;
                char *temp = (char *) realloc(buffer, capacity);
                if (nullptr == temp) {
                    free(buffer);
                    return nullptr;
                }
                buffer = temp;
            }
            unsigned long read_bytes = fread(buffer + offset, sizeof(char), portion, pipe_fp);
            if (ferror(pipe_fp)) {
                free(buffer);
                return nullptr;
            }
            if (0 == read_bytes) {
                break;
            } else {
                offset += read_bytes;
            }
        }
        buffer[offset] = '\0';
        return buffer;
    }
}
