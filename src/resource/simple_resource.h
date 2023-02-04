#ifndef PTHREAD_HTTP_PROXY_SIMPLE_RESOURCE_H
#define PTHREAD_HTTP_PROXY_SIMPLE_RESOURCE_H

#include <cassert>
#include <set>
#include <vector>
#include <unistd.h>
#include <sys/eventfd.h>

#include "resource.h"

#include "../utils/io_operations.h"
#include "../utils/log.h"
#include "../sync/threadsafe_list.h"

namespace multithread_proxy {

    class SimpleResource : public Resource {
    public:
        SimpleResource() {
            event_fd = eventfd(0, EFD_SEMAPHORE);
            if (event_fd == -1) {
                logErrorWithErrno("Error in eventfd()");
                assert(false);
            }
            rwlock = new pthread_rwlock_t;
            int code = pthread_rwlock_init(rwlock, nullptr);
            assert(code >= 0);
            parts = new ThreadsafeList<io::Message>(rwlock);
        }

        ~SimpleResource() override {
            delete full_data;
            if (free_messages) {
                size_t size = parts->size();
                for (size_t i = 0; i < size; i++) {
                    delete parts->at(i);
                }
            }
            delete parts;
            pthread_rwlock_destroy(rwlock);
            delete rwlock;
        }

        int getNotifyFd() override {
            return event_fd;
        }

        int subscribe() override {
            pthread_rwlock_wrlock(rwlock);
            subscribes_count++;
            pthread_rwlock_unlock(rwlock);
            return event_fd;
        }

        int cancel() override {
            pthread_rwlock_wrlock(rwlock);
            if (subscribes_count > 0) subscribes_count--;
            pthread_rwlock_unlock(rwlock);
            return event_fd;
        }

        size_t getSubscribesCount() override {
            return subscribes_count;
        }

        List<io::Message> *getParts() override {
            return parts;
        }

        void setStatus(ResourceStatus r) override {
            status = r;
        }

        ResourceStatus getStatus() override {
            return status;
        }

        void setContentLength(size_t new_content_length) override {
            this->content_length = new_content_length;
        }

        size_t getContentLength() override {
            return content_length;
        }

        void setCurrentLength(size_t new_current_length) override {
            this->current_length = new_current_length;
        }

        size_t getCurrentLength() override {
            return current_length;
        }

        io::Message *getData() override {
            return full_data;
        }

        void updateData(io::Message *data) override {
            full_data = data;
        }

    private:
        int event_fd;
        ResourceStatus status = INCOMPLETED;
        List<io::Message> *parts;
        io::Message *full_data = nullptr;
        size_t current_length = 0;
        size_t content_length = 0;
        bool free_messages = true;
        pthread_rwlock_t *rwlock;

        size_t subscribes_count = 0;
        // vector of socket descriptors of clients who wait for the resource
    };
}

#endif //PTHREAD_HTTP_PROXY_SIMPLE_RESOURCE_H
