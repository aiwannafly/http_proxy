#include "server.h"

#include <cassert>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include "../status_code.h"
#include "../utils/socket_operations.h"
#include "../utils/log.h"
#include "../../httpparser/src/httpparser/httpresponseparser.h"

namespace multithread_proxy {
    const int HTTP_PORT = 80;

    void *RunNewServer(void *arg) {
        auto sArg = (ServerArgs *) arg;
        auto server = new Server(sArg->resource,
                                 sArg->hostname, sArg->rm);
        delete sArg;
        long code = server->run();
        delete server;
        pthread_exit((void *) code);
    }

    Server::Server(Resource *resource, const std::string &hostname,
                   io::Message *rm) {
        assert(resource);
        assert(rm);
        this->resource = resource;
        this->hostname = hostname;
        this->request_message = rm;
        this->notify_fd = resource->getNotifyFd();
    }

    Server::~Server() = default;

    int Server::run() {
        int code = connectToServer();
        if (code == status_code::FAIL) {
            return status_code::FAIL;
        }
        log("Connected to server " + std::to_string(fd));
        assert(request_message);
        bool written = io::WriteAll(fd, request_message);
        if (!written) {
            logErrorWithErrno("Could not send request to server");
            freeResources();
            return status_code::FAIL;
        }
        log("Sent request ");
        while (true) {
            auto message = io::ReadAll(fd);
            if (message == nullptr) {
                logErrorWithErrno("Could not read message from server");
                freeResources();
                return status_code::FAIL;
            }
            if (message->len == 0) {
                log("Got empty response from server");
                freeResources();
                return status_code::FAIL;
            }
            code = handleResponse(message);
            if (code == status_code::FAIL) {
                logErrorWithErrno("Failed to handle response");
                freeResources();
                return status_code::FAIL;
            } else if (code == status_code::COMPLETED) {
                log("Server " + std::to_string(fd) + " completed downloading");
                freeResources();
                return status_code::FAIL;
            }
        }
    }

    int Server::connectToServer() {
        int port = HTTP_PORT;
        int sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == status_code::FAIL) {
            return status_code::FAIL;
        }
        struct hostent *hostnm;
        hostnm = gethostbyname(hostname.data());
        if (hostnm == nullptr) {
            logErrorWithErrno("Failed to get by hostname");
            return status_code::FAIL;
        }
        struct sockaddr_in serv_sockaddr{};
        serv_sockaddr.sin_family = AF_INET;
        serv_sockaddr.sin_port = htons(port);
        serv_sockaddr.sin_addr.s_addr = *((unsigned long *) hostnm->h_addr);
        int return_code = connect(sd, (const struct sockaddr *) &serv_sockaddr, sizeof(serv_sockaddr));
        if (return_code < 0) {
            close(sd);
            return status_code::FAIL;
        }
        fd = sd;
        return sd;
    }

    int Server::notifySubscribers() const {
        size_t post_count = resource->getSubscribesCount();
        if (post_count == 0) post_count = 1;
        int code = eventfd_write(notify_fd, post_count);
        return code;
    }

    int Server::handleResponse(io::Message *new_part) {
        if (new_part->len == 0) {
            delete new_part;
            return status_code::SUCCESS;
        }
        resource->getParts()->add(new_part);
        io::Message *full_msg = new_part;
        if (resource->getContentLength() == 0) {
            if (resource->getData() != nullptr) {
                full_msg = resource->getData();
                bool added = io::AppendMsg(resource->getData(), new_part);
                if (!added) {
                    return status_code::FAIL;
                }
            } else {
                resource->updateData(io::copy(new_part));
            }
        }
        resource->setCurrentLength(resource->getCurrentLength() + new_part->len);
//        log("Server " + std::to_string(fd) + " received " + std::to_string(resource->getCurrentLength()) +
//            "/" + std::to_string(resource->getContentLength()) + " bytes");
        if (resource->getContentLength() > resource->getCurrentLength()) {
            return notifySubscribers();
        } else if (resource->getContentLength() == resource->getCurrentLength()
                   && resource->getCurrentLength() > 0) {
            resource->setStatus(COMPLETED);
            delete resource->getData();
            resource->updateData(nullptr);
            notifySubscribers();
            return status_code::COMPLETED;
        }
        httpparser::Response response;
        httpparser::HttpResponseParser parser;
        httpparser::HttpResponseParser::ParseResult res = parser.parse(response, full_msg->data,
                                                                       full_msg->data + full_msg->len);
        log("Parsed response");
        if (res == httpparser::HttpResponseParser::ParsingError) {
            logError("Failed to parse response of " + hostname);
            return status_code::FAIL;
        }
        notifySubscribers();
        if (res == httpparser::HttpResponseParser::ParsingIncompleted) {
            auto content_length_header = std::find_if(response.headers.begin(), response.headers.end(),
                                                      [&](const httpparser::Response::HeaderItem &item) {
                                                          return item.name == "Content-Length";
                                                      });
            if (content_length_header != response.headers.end()) {
                size_t content_length = std::stoul(content_length_header->value);
                resource->setContentLength(content_length);
                assert(full_msg->len > response.content.size());
                size_t sub = full_msg->len - response.content.size();
                if (sub > 0) {
                    resource->setContentLength(content_length + sub);
                }
                log("Content-Length : " + std::to_string(resource->getContentLength()));
            }
            log("Response of is not complete, it's current length: " +
                std::to_string(full_msg->len));
            assert(resource->getStatus() == INCOMPLETED);
            return status_code::SUCCESS;
        }
        log("Parsed response of " + hostname + std::string(" code: ") + std::to_string(response.statusCode));
        resource->setStatus(COMPLETED);
        log("Full bytes length : " + std::to_string(resource->getData()->len));
        log("Full parts count : " + std::to_string(resource->getParts()->size()));
        delete resource->getData();
        resource->updateData(nullptr);
        if (response.statusCode != 200) {
            log("Status code is not 200, not store " + hostname + " in cache");
        }
        return status_code::COMPLETED;
    }

    void Server::freeResources() const {
        log("Terminate server thread " + std::to_string(fd));
        int ret_val = close(fd);
        if (ret_val < 0) {
            logErrorWithErrno("Error in close");
        }
    }
}
