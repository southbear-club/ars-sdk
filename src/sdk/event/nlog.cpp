#include "ars/sdk/event/nlog.hpp"
#include "ars/sdk/ds/list.hpp"
#include "ars/sdk/lock/lock.hpp"
#include "ars/sdk/macros/defs.hpp"
#include "ars/sdk/memory/mem.hpp"
#include "ars/sdk/net/sock.hpp"

namespace ars {

namespace sdk {

namespace event {

typedef struct network_logger_s {
    loop_t* loop;
    io_t* listenio;
    struct list_head clients;
} network_logger_t;

typedef struct nlog_client {
    io_t* io;
    struct list_node node;
} nlog_client;

static network_logger_t s_logger;
static mutex_lock_t s_mutex;

static void on_close(io_t* io) {
    printd("on_close fd=%d error=%d\n", io_fd(io), io_error(io));

    nlog_client* client = (nlog_client*)ars_event_userdata(io);
    if (client) {
        ars_event_set_userdata(io, NULL);

        mutex_lock(&s_mutex);
        list_del(&client->node);
        mutex_unlock(&s_mutex);

        ARS_FREE(client);
    }
}

static void on_read(io_t* io, void* buf, int readbytes) {
    printd("on_read fd=%d readbytes=%d\n", io_fd(io), readbytes);
    printd("< %s\n", (char*)buf);
    // nothing to do
}

static void on_accept(io_t* io) {
    /*
    printd("on_accept connfd=%d\n", io_fd(io));
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printd("accept connfd=%d [%s] <= [%s]\n", io_fd(io),
            SOCKADDR_STR(io_localaddr(io), localaddrstr),
            SOCKADDR_STR(io_peeraddr(io), peeraddrstr));
    */

    io_setcb_read(io, on_read);
    io_setcb_close(io, on_close);
    io_read(io);

    // free on_close
    nlog_client* client;
    ARS_ALLOC_SIZEOF(client);
    client->io = io;
    ars_event_set_userdata(io, client);

    mutex_lock(&s_mutex);
    list_add(&client->node, &s_logger.clients);
    mutex_unlock(&s_mutex);
}

void network_logger(int loglevel, const char* buf, int len) {
    struct list_node* node;
    nlog_client* client;

    mutex_lock(&s_mutex);
    list_for_each(node, &s_logger.clients) {
        client = list_entry(node, nlog_client, node);
        io_write(client->io, buf, len);
    }
    mutex_unlock(&s_mutex);
}

io_t* nlog_listen(loop_t* loop, int port) {
    s_logger.loop = loop;
    s_logger.listenio = loop_create_tcp_server(loop, "0.0.0.0", port, on_accept);
    list_init(&s_logger.clients);
    mutex_lock_init(&s_mutex);
    return s_logger.listenio;
}

}  // namespace event

}  // namespace sdk

}  // namespace ars
