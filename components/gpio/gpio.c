#include <lexp.h>
#include <module/module.h>
#include <module/error/mk_error.h>
#include <primitive.h>
#include <stdio.h>

lexp car(lexp p);
lexp cdr(lexp p);
lexp evlis(lexp t, lexp e);
extern lexp nil;
extern lexp tru;

int gpio_get_level(int gpio_num);
int gpio_set_level(int gpio_num, int level);
int gpio_set_direction(int gpio_num, int mode);
lexp f_get_gpio(lexp t, lexp e) {
        t = evlis(t, e);
	/* TODO: verify car(t) is a number in the range */
	int pin = car(t);
	if( gpio_get_level( pin ) )
		return tru;
	else
		return nil;
}

lexp f_set_gpio(lexp t, lexp e) {
        t = evlis(t, e);
	/* TODO: verify car(t) and car(cdr(t)) is a number in the range */
	int pin = car(t);
	int level = car(cdr(t));
	if( gpio_set_level( pin, level ) == 0 )
		return tru;
	/* TODO: return an error object */
	return nil;
}

lexp f_set_dir_gpio(lexp t, lexp e) {
        t = evlis(t, e);
	/* TODO: verify car(t) and car(cdr(t)) is a number in the range */
	int pin = car(t);
	int mode = car(cdr(t));
	if( gpio_set_direction( pin, mode ) == 0 )
		return tru;
	/* TODO: return an error object */
	return nil;
}



PRIMITIVE_SECTION struct primitive gpio_primitive[] = {
        {"set-gpio!",f_set_gpio},
        {"get-gpio", f_get_gpio},
        {"set-dir-gpio", f_set_dir_gpio},
};

static void init()
{
}

MODULE_SECTION struct module gpio = {
        .setup = init,
};
