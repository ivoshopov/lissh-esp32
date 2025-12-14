#include <io.h>
#include <module/error/mk_error.h>
#include <string.h>
#include "driver/gpio.h"




bool not(lexp x);
extern lexp nil, tru;
char* unbox_atom(lexp atom);
lexp assoc_pair(lexp v, lexp e);
lexp atom(const char *s);
lexp car(lexp p);
lexp cdr(lexp p);


static lexp gpioctl_write(struct io_typ *port, lexp exp) {
        esp_err_t rc;
        lexp id;
        lexp id_pair = assoc_pair(atom("id"), exp);
        if (typof(id_pair) != NIL) {
                id = car(cdr(id_pair));
                if (id >= GPIO_NUM_MAX)
                        return mk_error("Max id: ", GPIO_NUM_MAX);
        } else {
                return mk_error("Missing gpio id pair in: ", exp);
        }
        lexp do_val;
        lexp do_pair = assoc_pair(atom("do"), exp);
        if (typof(do_pair) != NIL) {
                do_val = car(cdr(do_pair));
        } else {
                return mk_error("Missing gpio do pair in: ", exp);
        }
        if (typof(do_val) != ATOM ) {
                return mk_error("Only ATOM is accepted as do pair", nil);
        }
        if (strcmp(unbox_atom(do_val), "get") == 0) {
                /* TODO: verify the id is in the GPIO number range */
                if( gpio_get_level( id ) )
                        return tru;
                return nil;
        } else if (strcmp(unbox_atom(do_val), "set") == 0) {
                lexp level_val;
                lexp level_pair = assoc_pair(atom("level"), exp);
                if (typof(level_pair) != NIL) {
                        level_val = car(cdr(level_pair));
                } else {
                        return mk_error("Missing level pair in: ", exp);
                }
                if (gpio_set_level( id, level_val ) == 0)
                        return nil;
                /* TODO: add gpio number in the error message */
                return mk_error("Couldn't set gpio level", nil);
        } else if (strcmp(unbox_atom(do_val), "set-dir") == 0) {
                lexp dir_val;
                lexp dir_pair = assoc_pair(atom("dir"), exp);
                if (typof(dir_pair) != NIL) {
                        dir_val = car(cdr(dir_pair));
                } else {
                        return mk_error("Missing dir pair in: ", exp);
                }
                if (typof(dir_val) == ATOM) {
                        if ( strcmp(unbox_atom(dir_val), "out") == 0 )
                                rc = gpio_set_direction( id, GPIO_MODE_OUTPUT );
                        else if ( strcmp(unbox_atom(dir_val), "in") == 0 )
                                rc = gpio_set_direction( id, GPIO_MODE_INPUT );
                        else if ( strcmp(unbox_atom(dir_val), "inout") == 0 )
                                rc = gpio_set_direction( id, GPIO_MODE_INPUT_OUTPUT );
                        else if ( strcmp(unbox_atom(dir_val), "disable") == 0 )
                                rc = gpio_set_direction( id, GPIO_MODE_DISABLE );
                        else if ( strcmp(unbox_atom(dir_val), "out-od") == 0 )
                                rc = gpio_set_direction( id, GPIO_MODE_OUTPUT_OD );
                        else if ( strcmp(unbox_atom(dir_val), "inout-od") == 0 )
                                rc = gpio_set_direction( id, GPIO_MODE_INPUT_OUTPUT_OD );
                        else return mk_error("Unknown value in dir pair", nil);
                        if( rc == ESP_OK )
                                return nil;
                        else
                                /* TODO: convert ESP error code to Lisp error */
                                return mk_error("Could not set GPIO direction", nil);
                } else {
                        return mk_error("The dir value must be an ATOM", nil);
                }
        } else if (strcmp(unbox_atom(do_val), "help") == 0) {
                return mk_error("Not implemented", nil);
        }
        return mk_error("Unknown value in do pair:", do_val);
}

PORTS_SECTION struct io_typ gpioctl_port = {
        .private = NULL,
        .read = NULL,
        .write = gpioctl_write,
        .proto = "gpioctl",
};
