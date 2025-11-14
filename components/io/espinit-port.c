#include <module/io/file.h>
#include <module/io/codec/text.h>

static char init_data[] = "\
 (define begin \
  (lambda (x . args) \
   (if args (begin . args) x))) \
 (write-to! 'uart1 \
   (eval \
    (read-from! 'init-script.lisp))) \
 (loop! \
  (write-to! 'uart1 \
   (eval \
    (read-from! 'uart1))))";

static struct file_typ espinit_buff = {
  .data = init_data,
  .pos = 0,
  .size = sizeof(init_data),
};


static struct io_primitive espinit_io = {
  .read = file_read,
  .write = NULL,
  .private = (void*)&espinit_buff,
};


PORTS_SECTION struct io_typ espinit_port = {
  .private = &espinit_io,
  .read = text_read,
  .write = NULL,
  .proto = "espinit.lisp",
};
