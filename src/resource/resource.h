#ifndef PTHREAD_HTTP_PROXY_RESOURCE_H
#define PTHREAD_HTTP_PROXY_RESOURCE_H


#include "../sync/list.h"
#include "../utils/io_operations.h"

namespace multithread_proxy {
    enum ResourceStatus {
        BAD, INCOMPLETED, COMPLETED
    };

    class Resource {
    public:
        virtual ~Resource() = default;

        virtual int getNotifyFd() = 0;

        virtual int subscribe() = 0;

        virtual int cancel() = 0;

        virtual size_t getSubscribesCount() = 0;

        virtual io::Message *getData() = 0;

        virtual void updateData(io::Message *data) = 0;

        virtual List<io::Message> *getParts() = 0;

        virtual void setStatus(ResourceStatus r) = 0;

        virtual ResourceStatus getStatus() = 0;

        virtual void setContentLength(size_t content_length) = 0;

        virtual size_t getContentLength() = 0;

        virtual void setCurrentLength(size_t current_length) = 0;

        virtual size_t getCurrentLength() = 0;
    };
}

#endif //PTHREAD_HTTP_PROXY_RESOURCE_H
