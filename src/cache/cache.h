#ifndef HTTP_CPP_PROXY_CACHE_H
#define HTTP_CPP_PROXY_CACHE_H

#include <string>
#include <functional>

template<class T>
class Cache {
public:
    virtual ~Cache() = default;

    virtual T *get(const std::string &) = 0;

    virtual void erase(const std::string &) = 0;

    virtual bool contains(const std::string &) = 0;

    virtual bool put(const std::string &, T *value) = 0;

    virtual size_t size() = 0;

    virtual size_t sizeBytes(std::function<size_t(const T *)> func) = 0;

    virtual void clear() = 0;
};


#endif //HTTP_CPP_PROXY_CACHE_H
