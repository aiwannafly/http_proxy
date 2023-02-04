#ifndef PTHREAD_HTTP_PROXY_CACHING_RESOURCE_MANAGER_H
#define PTHREAD_HTTP_PROXY_CACHING_RESOURCE_MANAGER_H

#include "resource_manager.h"
#include "../cache/cache.h"
#include <vector>

namespace multithread_proxy {

    class CachingResourceManager : public ResourceManager {
    public:

        explicit CachingResourceManager();

        ~CachingResourceManager() override;

        multithread_proxy::Resource *getResource(const std::string &name,
                                                 io::Message *http_request,
                                                 const std::string &hostname) override;

        void clear() override;

    private:
        pthread_mutex_t *mutex;
        Cache<Resource> *cache;
        std::vector<pthread_t> server_tids = std::vector<pthread_t>();
    };
}

#endif //PTHREAD_HTTP_PROXY_CACHING_RESOURCE_MANAGER_H
