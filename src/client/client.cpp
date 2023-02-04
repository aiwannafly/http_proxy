#include "client.h"

#include <sys/eventfd.h>

#include "../utils/log.h"
#include "../status_code.h"
#include "../../httpparser/src/httpparser/httprequestparser.h"

namespace multithread_proxy {

    void *RunNewClient(void *arg) {
        auto sArg = (ClientArgs *) arg;
        auto client = new Client(sArg->fd, sArg->rm);
        delete sArg;
        long code = client->run();
        delete client;
        pthread_exit((void *) code);
    }

    Client::Client(int fd, ResourceManager *rm) {
        this->fd = fd;
        this->resource_manager = rm;
    }

    Client::~Client() = default;

    int Client::run() {
        //Well, so. In this connection we will handle requests from client
        log("Run client thread " + std::to_string(fd));
        int code = handleRequest();
        if (code == status_code::FAIL) {
            logError("Failed to handle request");
            freeResources();
            return status_code::FAIL;
        } else if (code == status_code::TERMINATE) {
            logError("Terminate");
            freeResources();
            return status_code::COMPLETED;
        } else if (code == status_code::COMPLETED) {
            log("Client " + std::to_string(fd) + " completed receiving, got " + std::to_string(recv_bytes) + " bytes");
            freeResources();
            return status_code::COMPLETED;
        }
        while (true) {
            eventfd_t value;
            code = eventfd_read(notify_fd, &value);
            if (code == status_code::FAIL) {
                logErrorWithErrno("Error in eventfd_read()");
                freeResources();
                return status_code::FAIL;
            }
            code = sendAvailableParts();
            if (code == status_code::FAIL) {
                logErrorWithErrno("Error in sendParts()");
                freeResources();
                return status_code::FAIL;
            }
            if (resource->getStatus() == COMPLETED && resource->getParts()->size() == recv_count) {
                log("Client " + std::to_string(fd) + " completed receiving, got " + std::to_string(recv_bytes) +
                    " bytes");
                freeResources();
                return status_code::COMPLETED;
            }
            if (resource->getStatus() == COMPLETED) {
                log("Resource is completed, but client " + std::to_string(fd) + " didn't get all chunks");
            }
        }
    }

    int Client::sendAvailableParts() {
       while (recv_count < resource->getParts()->size()) {
            auto *msg = resource->getParts()->at(recv_count);
            recv_count++;
            recv_bytes += msg->len;
            bool written = io::WriteAll(fd, msg);
            if (!written) {
                return status_code::FAIL;
            }
        }
        return status_code::SUCCESS;
    }

    void Client::freeResources() const {
        log("Terminate client thread " + std::to_string(fd));
        if (resource != nullptr) {
            resource->cancel();
        }
        int ret_val = close(fd);
        if (ret_val < 0) {
            logErrorWithErrno("Error in close");
        }
    }

    int Client::handleRequest() {
        io::Message *message = io::ReadAll(fd);
        if (message == nullptr) {
            return status_code::FAIL;
        }
        if (message->len == 0) {
            delete message;
            return status_code::TERMINATE;
        }
        httpparser::Request request;
        httpparser::HttpRequestParser parser;
        httpparser::HttpRequestParser::ParseResult res = parser.parse(request,
                                                                      message->data,
                                                                      message->data + message->len);
        if (res != httpparser::HttpRequestParser::ParsingCompleted) {
            delete message;
            return status_code::FAIL;
        }
        log(request.method);
        log(request.uri);
        log(std::string("HTTP Version is 1." + std::to_string(request.versionMinor)));
        std::string res_name = request.uri;
        if (request.method != "GET") {
            delete message;
            return status_code::FAIL;
        }
        for (const httpparser::Request::HeaderItem &header: request.headers) {
            log(header.name + std::string(" : ") + header.value);
            if (header.name == "Host") {
                resource = resource_manager->getResource(res_name,
                                                         message,
                                                         header.value);
                if (resource == nullptr) {
                    delete message;
                    return status_code::FAIL;
                } else {
                    if (resource->getStatus() == INCOMPLETED) {
                        sendAvailableParts();
                        notify_fd = resource->subscribe();
                        return status_code::SUCCESS;
                    } else if (resource->getStatus() == COMPLETED) {
                        sendAvailableParts();
                        return status_code::COMPLETED;
                    }
                }
            }
        }
        logError("Not found host header");
        delete message;
        return status_code::FAIL;
    }
}
