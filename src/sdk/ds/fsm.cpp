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
 * @file fsm.cpp
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
#include <stddef.h>
#include "ars/sdk/ds/fsm.hpp"
#include "ars/sdk/memory/mem.hpp"

namespace ars {
    
namespace sdk {

struct fsm *fsm_create()
{
    struct fsm *fsm = (struct fsm *)aru_calloc(1, sizeof(struct fsm));
    if (!fsm) {
        printf("malloc failed!\n");
        return NULL;
    }
    fsm->curr_state = 0;
    mutex_lock_init(&fsm->mutex);
    return fsm;
}

void fsm_destroy(struct fsm *fsm)
{
    if (fsm) {
        mutex_lock_deinit(&fsm->mutex);
        aru_free(fsm);
    }
}

int fsm_state_init(struct fsm *fsm, int state)
{
    fsm->curr_state = state;
    return 0;
}

int fsm_action(struct fsm *fsm, int event_id, void *args)
{
    int i;
    int ret;
    mutex_lock(&fsm->mutex);
    for (i = 0; i < fsm->table_num; ++i) {
        if (fsm->curr_state == fsm->table[i].current_state)
            break;
    }
    if (i == fsm->table_num) {
        printf("can not find state %d\n", fsm->curr_state);
        ret = -1;
        goto out;
    }
    if (event_id != fsm->table[i].trigger_event) {
        printf("invalid trigger_event[%d] from state[%d] to state[%d]\n", event_id, fsm->curr_state, fsm->table[i].next_state);
        ret = -1;
        goto out;
    }
    ret = fsm->table[i].do_action(args);
    printf("change state %d -> %d\n", fsm->curr_state, fsm->table[i].next_state);
    fsm->curr_state = fsm->table[i].next_state;
out:
    mutex_unlock(&fsm->mutex);
    return ret;
}

} // namespace sdk

} // namespace ars
