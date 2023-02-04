#ifndef PTHREAD_HTTP_PROXY_RESOURCE_MANAGER_H
#define PTHREAD_HTTP_PROXY_RESOURCE_MANAGER_H

#include "../resource/resource.h"

namespace multithread_proxy {
    class ResourceManager {
    public:

        virtual ~ResourceManager() = default;

        virtual multithread_proxy::Resource *getResource(const std::string &name,
                                                         io::Message *http_request,
                                                         const std::string &hostname) = 0;

        virtual void clear() = 0;
    };
}

#endif //PTHREAD_HTTP_PROXY_RESOURCE_MANAGER_H
