#include "caching_resource_manager.h"

#include "../resource/simple_resource.h"
#include "../cache/map_cache.h"
#include "../server/server.h"

namespace multithread_proxy {

    CachingResourceManager::CachingResourceManager() {
        cache = new MapCache<Resource>;
        mutex = new pthread_mutex_t;
        int code = pthread_mutex_init(mutex, nullptr);
        assert(code == 0);
    }

    CachingResourceManager::~CachingResourceManager() {
        delete cache;
        pthread_mutex_destroy(mutex);
        delete mutex;
    }

    Resource * multithread_proxy::CachingResourceManager::getResource(const std::string &name,
                                                                      io::Message *http_request,
                                                                      const std::string &hostname) {
        pthread_mutex_lock(mutex);
        if (cache->contains(name)) {
            delete http_request;
            pthread_mutex_unlock(mutex);
            return cache->get(name);
        }
        auto *res = new SimpleResource;
        cache->put(name, res);
        pthread_mutex_unlock(mutex);
        auto *arg = new ServerArgs;
        arg->rm = http_request;
        arg->resource = res;
        arg->hostname = hostname;
        pthread_t tid;
        int code = pthread_create(&tid, nullptr, RunNewServer, arg);
        if (code < 0) {
            delete res;
            return nullptr;
        }
        server_tids.push_back(tid);
        return res;
    }

    void CachingResourceManager::clear() {
        for (const auto &tid : server_tids) {
            int code = pthread_join(tid, nullptr);
            if (code < 0) {
                logErrorWithErrno("Error in join");
            }
        }
        cache->clear();
    }
}
