#include "aru/components/evpp/UdpServer.hpp"

using namespace aru::evpp;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return -10;
    }
    int port = atoi(argv[1]);

    UdpServer srv;
    int bindfd = srv.createsocket(port);
    if (bindfd < 0) {
        return -20;
    }
    printf("server bind on port %d, bindfd=%d ...\n", port, bindfd);
    srv.onMessage = [](const SocketChannelPtr& channel, Buffer* buf) {
        // echo
        printf("< %.*s\n", (int)buf->size(), (char*)buf->data());
        channel->write(buf);
    };
    srv.onWriteComplete = [](const SocketChannelPtr& channel, Buffer* buf) {
        printf("> %.*s\n", (int)buf->size(), (char*)buf->data());
    };
    srv.start();

    while (1) sleep(1);
    return 0;
}
