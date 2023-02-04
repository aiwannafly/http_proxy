#ifndef PTHREAD_HTTP_PROXY_THREADSAFE_LIST_H
#define PTHREAD_HTTP_PROXY_THREADSAFE_LIST_H

#include <cassert>
#include <cstddef>
#include <vector>
#include "pthread.h"

#include "list.h"

template <class T>
class ThreadsafeList : public List<T>{
public:
    explicit ThreadsafeList(pthread_rwlock_t *rwlock) {
        list = std::vector<T*>();
        this->rwlock = rwlock;
    }

    ~ThreadsafeList() override = default;

    T *at(size_t pos) const override {
        pthread_rwlock_rdlock(rwlock);
        auto res = list.at(pos);
        pthread_rwlock_unlock(rwlock);
        return res;
    }

    size_t size() const override {
        pthread_rwlock_rdlock(rwlock);
        auto res = list.size();
        pthread_rwlock_unlock(rwlock);
        return res;
    }

    void add(T *new_elem) override {
        pthread_rwlock_wrlock(rwlock);
        list.push_back(new_elem);
        pthread_rwlock_unlock(rwlock);
    }

private:
    std::vector<T*> list;
    pthread_rwlock_t *rwlock{};
};


#endif //PTHREAD_HTTP_PROXY_THREADSAFE_LIST_H
