set -e


export PATH=${PATH}:/home/ivo/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/
export CC=xtensa-esp32-elf-gcc
export AR=xtensa-esp32-elf-ar
export CFLAGS="-mlongcalls -Wno-frame-address  -fno-builtin-memcpy -fno-builtin-memset -fno-builtin-bzero -fno-builtin-stpcpy -fno-builtin-strncpy -ffunction-sections -fdata-sections -Wall -Werror=all -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-enum-conversion -gdwarf-4 -ggdb -Og -fno-shrink-wrap -fmacro-prefix-map=/home/ivo/projects/esp/repl-main=. -fmacro-prefix-map=/home/ivo/projects/esp/esp-idf=/IDF -fstrict-volatile-bitfields -fno-jump-tables -fno-tree-switch-conversion -std=gnu17 -Wno-old-style-declaration"
export CPPFLAGS="-DESP_PLATFORM -DIDF_VER=\"v5.3\" -DSOC_MMU_PAGE_SIZE=CONFIG_MMU_PAGE_SIZE -DSOC_XTAL_FREQ_MHZ=CONFIG_XTAL_FREQ -D_GLIBCXX_HAVE_POSIX_SEMAPHORE -D_GLIBCXX_USE_POSIX_SEMAPHORE -D_GNU_SOURCE -D_POSIX_READER_WRITER_LOCKS"

cd tinylisp
sed 	-e "s/CONFIG_MAIN=y/# CONFIG_MAIN=y/" \
	-e "s/CONFIG_DO_ELF=y/# CONFIG_DO_ELF=y/" \
	-e "s/# CONFIG_DO_LIB=y/CONFIG_DO_LIB=y/" \
	-e 's/CONFIG_DEFAULT_PORT="std"/CONFIG_DEFAULT_PORT="uart1"/' < defconfig > .config
make
