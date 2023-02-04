#ifndef PTHREAD_HTTP_PROXY_RUNNABLE_H
#define PTHREAD_HTTP_PROXY_RUNNABLE_H

class Runnable {
public:

    virtual ~Runnable() = default;

    virtual int run() = 0;

};

#endif //PTHREAD_HTTP_PROXY_RUNNABLE_H
