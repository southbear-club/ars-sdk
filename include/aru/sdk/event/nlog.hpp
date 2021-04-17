#pragma once

#include "loop.hpp"

namespace aru {

namespace sdk {
    
#define DEFAULT_LOG_PORT    10514

void network_logger(int loglevel, const char* buf, int len);
hio_t* nlog_listen(hloop_t* loop, int port);

}
}