#pragma once

#include "iowatcher.hpp"

#ifdef EVENT_IOCP

// #include "buf.h"
// #include "hsocket.h"
// #include <mswsock.h>

// typedef struct hoverlapped_s {
//     OVERLAPPED  ovlp;
//     int         fd;
//     int         event;
//     WSABUF      buf;
//     int         bytes;
//     int         error;
//     hio_t*      io;
//     // for recvfrom
//     struct sockaddr* addr;
//     int         addrlen;
// } hoverlapped_t;

// int post_acceptex(hio_t* listenio, hoverlapped_t* hovlp);
// int post_recv(hio_t* io, hoverlapped_t* hovlp);

#endif
