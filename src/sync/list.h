#ifndef PTHREAD_HTTP_PROXY_LIST_H
#define PTHREAD_HTTP_PROXY_LIST_H

#include <cstddef>

template <class T>
class List {
public:
    virtual ~List() = default;

    virtual void add(T *) = 0;

    virtual size_t size() const = 0;

    virtual T* at(size_t) const = 0;
};

#endif //PTHREAD_HTTP_PROXY_LIST_H
