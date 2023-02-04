#ifndef HTTP_CPP_PROXY_SELECT_DATA_H
#define HTTP_CPP_PROXY_SELECT_DATA_H

#include <cstdlib>

namespace io {
    class SelectData {
    public:
        enum fd_type {
            READ, WRITE
        };

        SelectData() {
            FD_ZERO(read_set);
            FD_ZERO(write_set);
        }

        ~SelectData() {
            delete read_set;
            delete write_set;
        }

        void addFd(int fd, fd_type type) {
            if (type == READ) {
                FD_SET(fd, read_set);
            } else {
                FD_SET(fd, write_set);
            }
            if (fd > max_fd) {
                max_fd = fd;
            }
        }

        void remove_fd(int fd, fd_type type) {
            if (type == READ) {
                FD_CLR(fd, read_set);
                if (fd == max_fd) {
                    max_fd--;
                }
            } else {
                FD_CLR(fd, write_set);
            }
        }

        [[nodiscard]] fd_set *getReadSet() const {
            return read_set;
        }

        [[nodiscard]] fd_set *getWriteSet() const {
            return write_set;
        }

        [[nodiscard]] int getMaxFd() const {
            return max_fd;
        }

    private:
        int max_fd = 0;
        fd_set *read_set = new fd_set;
        fd_set *write_set = new fd_set;
    };
}

#endif //HTTP_CPP_PROXY_SELECT_DATA_H
