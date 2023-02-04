#!/bin/bash
clang++ -Wall -pedantic -fsanitize=address -std=c++2a main.cpp multithread_proxy.cpp utils/socket_operations.cpp utils/io_operations.cpp utils/log.cpp server/server.cpp client/client.cpp resource_manager/caching_resource_manager.cpp -lpthread -o ../build/proxy
echo "Program build/proxy compiled"

