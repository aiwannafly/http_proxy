#ifndef HTTP_CPP_PROXY_SOCKET_OPERATIONS_H
#define HTTP_CPP_PROXY_SOCKET_OPERATIONS_H

#include <arpa/inet.h>

namespace socket_operations {

    int SetNonblocking(int serv_socket);

    int SetReusable(int serv_socket);

    int ConnectToAddress(char *serv_ipv4_address, int port);

    int ConnectToSockaddr(struct sockaddr_in *addr, int port);
}

#endif //HTTP_CPP_PROXY_SOCKET_OPERATIONS_H
