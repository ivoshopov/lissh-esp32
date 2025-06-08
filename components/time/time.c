#include <lexp.h>
#include <module/module.h>
#include <primitive.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern lexp nil;
lexp car(lexp p);
lexp cdr(lexp p);
lexp evlis(lexp t, lexp e);

lexp f_get_uptime(lexp t, lexp e) {
	return (lexp)xTaskGetTickCount() / configTICK_RATE_HZ;
}

lexp f_sleep(lexp t, lexp e) {
        t = evlis(t, e);
        /* TODO: check does t is a pair in order to call car in it */
        t = car(t);
        vTaskDelay(t * configTICK_RATE_HZ);
        return nil;
}


PRIMITIVE_SECTION struct primitive time_primitive[] = {
        {"get-uptime!", f_get_uptime},
        {"sleep", f_sleep},
};

static void init()
{
}

MODULE_SECTION struct module time = {
        .setup = init,
};
