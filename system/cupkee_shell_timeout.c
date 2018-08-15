/* GPLv2 License
 *
 * Copyright (C) 2016-2018 Lixing Ding <ding.lixing@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 **/

#include <cupkee.h>

#include "cupkee_shell_inner.h"

#define FLAG_TIMEOUT      (0x80)
#define FLAG_INTERVAL     (0x80 | CUPKEE_FLAG_REPEAT)

static void timeout_handle(int drop, void *param)
{
    if (drop) {
        shell_reference_release(param);
    } else {
        cupkee_execute_function(param, 0, NULL);
    }
}

static int timeout_register(int ac, val_t *av, int flags)
{
    val_t   *handle;
    uint32_t wait;
    cupkee_timeout_t *timeout;
    val_t *ref;

    if (ac < 1 || !val_is_function(av)) {
        return -1;
    }
    handle = av++;


    if (ac > 1 && val_is_number(av)) {
        wait = val_2_double(av);
    } else {
        wait = 0;
    }

    ref = shell_reference_create(handle);
    if (!ref) {
        return -1;
    }

    timeout = cupkee_timeout_register(wait, flags, timeout_handle, ref);
    if (!timeout) {
        shell_reference_release(ref);
        return -1;
    }

    return timeout->id;
}

static int timeout_unregister(int ac, val_t *av, int flags)
{
    int32_t tid = -1; // all

    if (ac > 0) {
        if (val_is_number(av)) {
            tid = val_2_intptr(av);
        } else {
            return -1;
        }
    }

    if (tid >= 0) {
        return cupkee_timeout_clear_with_id(tid);
    } else {
        return cupkee_timeout_clear_with_flags(flags);
    }
}

val_t native_set_timeout(env_t *env, int ac, val_t *av)
{
    int tid = timeout_register(ac, av, FLAG_TIMEOUT);

    (void) env;

    return tid < 0 ? val_mk_boolean(0) : val_mk_number(tid);
}

val_t native_set_interval(env_t *env, int ac, val_t *av)
{
    int tid = timeout_register(ac, av, FLAG_INTERVAL);
    (void) env;

    return tid < 0 ? val_mk_boolean(0) : val_mk_number(tid);
}

val_t native_update_interval(env_t *env, int ac, val_t *av)
{
    (void) env;

    if (ac > 1 && val_is_number(av) && val_is_number(av + 1)) {
        uint32_t id = val_2_integer(av);
        uint32_t wait = val_2_integer(av + 1);

        if (cupkee_timeout_update_with_id(id, wait) == 1) {
            return VAL_TRUE;
        }
    }

    return VAL_FALSE;
}

val_t native_clear_timeout(env_t *env, int ac, val_t *av)
{
    int n = timeout_unregister(ac, av, FLAG_TIMEOUT);

    (void) env;

    return n < 0 ? val_mk_boolean(0) : val_mk_number(n);
}

val_t native_clear_interval(env_t *env, int ac, val_t *av)
{
    int n = timeout_unregister(ac, av, FLAG_INTERVAL);

    (void) env;

    return n < 0 ? val_mk_boolean(0) : val_mk_number(n);
}

