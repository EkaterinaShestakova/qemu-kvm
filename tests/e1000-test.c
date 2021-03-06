/*
 * QTest testcase for e1000 NIC
 *
 * Copyright (c) 2013-2014 SUSE LINUX Products GmbH
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include <glib.h>
#include <string.h>
#include "libqtest.h"
#include "qemu/osdep.h"

/* Tests only initialization so far. TODO: Replace with functional tests */
static void test_device(gconstpointer data)
{
    const char *model = data;
    QTestState *s;
    char *args;

    args = g_strdup_printf("-device %s", model);
    s = qtest_start(args);

    if (s) {
        qtest_quit(s);
    }
    g_free(args);
}

static const char *models[] = {
    "e1000",
    "e1000-82540em",
#if 0 /* Disabled in Red Hat Enterprise Linux 7 */
    "e1000-82544gc",
    "e1000-82545em",
#endif
};

int main(int argc, char **argv)
{
    int i;

    g_test_init(&argc, &argv, NULL);

    for (i = 0; i < ARRAY_SIZE(models); i++) {
        char *path;

        path = g_strdup_printf("/%s/e1000/%s", qtest_get_arch(), models[i]);
        g_test_add_data_func(path, models[i], test_device);
    }

    return g_test_run();
}
