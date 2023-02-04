#ifndef HTTP_CPP_PROXY_MAP_CACHE_H
#define HTTP_CPP_PROXY_MAP_CACHE_H

#include <map>

#include "cache.h"

template<class T>
class MapCache final : public Cache<T> {
public:

    MapCache() {
        this->table = new std::map<std::string, T *>();
    }

    ~MapCache() {
        delete table;
    }

    T *get(const std::string &key) override {
        return table->at(key);
    }

    void erase(const std::string &key) override {
        table->erase(key);
    }

    bool contains(const std::string &key) override {
        return table->contains(key);
    }

    bool put(const std::string &key, T *value) override {
        (*table)[key] = value;
        return true;
    };

    size_t size() override {
        return table->size();
    }

    size_t sizeBytes(std::function<size_t(const T *)> size) override {
        size_t total_size = 0;
        for (auto it = table->begin(); it != table->end(); it++) {
            total_size += size(it->second);
        }
        return total_size;
    }

    void clear() override {
        for (auto it = table->begin(); it != table->end(); it++) {
            delete it->second;
        }
        table->clear();
    }

private:
    std::map<std::string, T *> *table;
};

#endif //HTTP_CPP_PROXY_MAP_CACHE_H
