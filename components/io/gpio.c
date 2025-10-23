#include <io.h>
#include <module/error/mk_error.h>
#include <string.h>
#include "driver/gpio.h"



struct gpio_desc {
        int pin;
};

static struct gpio_desc gpio0 = { 0 };

bool not(lexp x);
extern lexp nil, tru;
char* unbox_atom(lexp atom);

static lexp gpio_write(struct io_typ *port, lexp exp) {
        struct gpio_desc *gpio = port->private;
        int level = 1;
        if( not(exp) )
                level = 0;
        if( gpio_set_level( gpio->pin, level ) == 0 )
                return nil;
        /* TODO: add gpio number in the error message */
        return mk_error("Couldn't set gpio: ", nil);
}

static lexp gpio_read(struct io_typ *port) {
        struct gpio_desc *gpio = port->private;
	if( gpio_get_level( gpio->pin ) )
		return tru;
        return nil;
}

static lexp gpio_dir_write(struct io_typ *port, lexp exp) {
        struct gpio_desc *gpio = port->private;
        esp_err_t rc;
        if( typof(exp) == ATOM ) {
                if ( strcmp(unbox_atom(exp), "out") == 0 )
                        rc = gpio_set_direction( gpio->pin, GPIO_MODE_OUTPUT );
                else if ( strcmp(unbox_atom(exp), "in") == 0 )
                        rc = gpio_set_direction( gpio->pin, GPIO_MODE_INPUT );
                else if ( strcmp(unbox_atom(exp), "inout") == 0 )
                        rc = gpio_set_direction( gpio->pin, GPIO_MODE_INPUT_OUTPUT );
                else if ( strcmp(unbox_atom(exp), "disable") == 0 )
                        rc = gpio_set_direction( gpio->pin, GPIO_MODE_DISABLE );
                else if ( strcmp(unbox_atom(exp), "out-od") == 0 )
                        rc = gpio_set_direction( gpio->pin, GPIO_MODE_OUTPUT_OD );
                else if ( strcmp(unbox_atom(exp), "inout-od") == 0 )
                        rc = gpio_set_direction( gpio->pin, GPIO_MODE_INPUT_OUTPUT_OD );
                else return mk_error("Unknown GPIO mode", nil);
                if( rc == ESP_OK )
                        return nil;
                else
                        /* TODO: convert ESP error code to Lisp error */
                        return mk_error("Could not set GPIO mode", nil);
        }
        return mk_error("Only ATOM is accepted as mode of gpio", nil);
}

PORTS_SECTION struct io_typ gpio0_port = {
        .private = &gpio0,
        .read = gpio_read,
        .write = gpio_write,
        .proto = "gpio0",
};

PORTS_SECTION struct io_typ gpio0_dir_port = {
        .private = &gpio0,
        .read = NULL,
        .write = gpio_dir_write,
        .proto = "gpio0-dir",
};
