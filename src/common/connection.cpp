#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "connection.h"
#include "exception.h"

Connection::Connection(int sd, struct sockaddr_in6 peer) {
    this->sd = sd;
    this->peer = peer;
}

Connection::Connection(const char* hostname, uint16_t port) {
    // do everything is needed to connect
    if ((this->sd = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
        throw ExSocket("can not create socket() for new Connection()", errno);
    }
    memset(&peer, 0, sizeof(peer));
    peer.sin6_family = AF_INET6;
    peer.sin6_port = htons(port);
    inet_pton(AF_INET6, hostname, &peer.sin6_addr);

    if (::connect(this->sd, (struct sockaddr*)&peer, sizeof(peer)) == -1) {
        throw ExConnect("can not connect() for new Connection()", errno);
    };

}

ssize_t Connection::send(const char* buffer, size_t len) {
    int ret = ::send(this->sd, (void*)buffer, len, 0);
    if (ret <= 0) {
        throw ExSend("can not send()");
    }
    return ret;
}

ssize_t Connection::recv(char* buffer, size_t len) {
    int ret = ::recv(this->sd, (void*)buffer, len, 0);
    if (ret <= 0) {
        throw ExRecv("can not recv()");
    }
    return ret;
}

Connection::~Connection() {
    clog << "destroy " << sd << endl;
    // close connections and destroy created sockets, if necessary
    // (eg. if socket was created using this->connect() )
}

