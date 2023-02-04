#ifndef HTTP_CPP_PROXY_SINGLE_THREAD_PROXY_H
#define HTTP_CPP_PROXY_SINGLE_THREAD_PROXY_H

#include <cstdlib>
#include <map>
#include <set>
#include <vector>

#include "proxy.h"
#include "utils/io_operations.h"
#include "utils/select_data.h"

#include "../httpparser/src/httpparser/httpresponseparser.h"
#include "../httpparser/src/httpparser/response.h"
#include "resource_manager/resource_manager.h"

namespace multithread_proxy {

    class HttpProxy final : public Proxy {
    public:

        explicit HttpProxy(bool print_allowed);

        ~HttpProxy() final;

        void run(int port) final;

    private:

        int initAndBindProxySocket(int port);

        int acceptNewClient();

        void freeResources();

        int proxy_socket = 0;
        ResourceManager *resource_manager;
        std::vector<pthread_t> client_tids = std::vector<pthread_t>();
    };
}

#endif //HTTP_CPP_PROXY_SINGLE_THREAD_PROXY_H
