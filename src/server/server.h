#ifndef PTHREAD_HTTP_PROXY_SERVER_H
#define PTHREAD_HTTP_PROXY_SERVER_H

#include <string>

#include "../resource/resource.h"
#include "../runnable.h"
#include "../utils/select_data.h"

namespace multithread_proxy {

    class Server : Runnable {
    public:
        Server(Resource *resource, const std::string &hostname,
               io::Message *rm);

        ~Server() override;

        int run() override;

    private:
        void freeResources() const;

        int connectToServer();

        int handleResponse(io::Message *new_part);

        int notifySubscribers() const;

        int fd = -1;
        int notify_fd = -1;
        std::string hostname;
        io::Message *request_message;
        Resource *resource;
    };

    typedef struct {
        Resource *resource;
        std::string hostname;
        io::Message *rm;
    } ServerArgs;

    void *RunNewServer(void *arg);
}

#endif //PTHREAD_HTTP_PROXY_SERVER_H
