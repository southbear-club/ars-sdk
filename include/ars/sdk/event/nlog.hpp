#pragma once

#include "loop.hpp"

namespace ars {

namespace sdk {

namespace event {

#define ARS_NLOG_DEFAULT_LOG_PORT 10514

void network_logger(int loglevel, const char* buf, int len);
io_t* nlog_listen(loop_t* loop, int port);

}  // namespace event

}  // namespace sdk

}  // namespace ars
