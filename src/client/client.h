#ifndef PTHREAD_HTTP_PROXY_CLIENT_H
#define PTHREAD_HTTP_PROXY_CLIENT_H

#include <unistd.h>

#include "../cache/cache.h"
#include "../runnable.h"
#include "../utils/select_data.h"
#include "../resource_manager/resource_manager.h"
#include "../utils/io_operations.h"

namespace multithread_proxy {
    class Client : public Runnable {
    public:
        Client(int fd, ResourceManager *rm);

        ~Client() override;

        int run() override;

    private:
        void freeResources() const;

        int handleRequest();

        int sendAvailableParts();

        int fd = -1;
        int notify_fd = -1;
        size_t recv_count = 0;
        size_t recv_bytes = 0;
        Resource *resource = nullptr;
        ResourceManager *resource_manager;
    };

    typedef struct {
        int fd;
        ResourceManager *rm;
    } ClientArgs;

    void *RunNewClient(void *arg);
}

#endif //PTHREAD_HTTP_PROXY_CLIENT_H
