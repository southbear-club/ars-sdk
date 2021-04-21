/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file shm.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "aru/sdk/ipc/ipc.hpp"

namespace aru {
    
namespace sdk {

struct shm_ctx {

};


static void *shm_init(ipc_t *ipc, uint16_t port, enum ipc_role role)
{
    return NULL;
}


static void shm_deinit(ipc_t *ipc)
{

}


static int shm_write(ipc_t *ipc, const void *buf, size_t len)
{
    return 0;
}

static int shm_read(ipc_t *ipc, void *buf, size_t len)
{
    return 0;
}


struct ipc_ops shm_ops = {
    .init             = shm_init,
    .deinit           = shm_deinit,
    .accept           = NULL,
    .connect          = NULL,
    .register_recv_cb = NULL,
    .send             = shm_write,
    .recv             = shm_read,
};

} // namespace sdk

} // namespace aru
