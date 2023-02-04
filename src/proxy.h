#ifndef HTTP_CPP_PROXY_PROXY_H
#define HTTP_CPP_PROXY_PROXY_H

class Proxy {
public:

    virtual ~Proxy() = default;

    virtual void run(int port) = 0;

};

#endif //HTTP_CPP_PROXY_PROXY_H
