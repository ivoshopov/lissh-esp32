
BUILD_DIR ?= build

all: $(BUILD_DIR)/.dir
	sh esp32-defconfig.sh
	cd $(BUILD_DIR) && cmake -DPYTHON_DEPS_CHECKED=1 -DESP_PLATFORM=1 -DCCACHE_ENABLE=0 .. && make

$(BUILD_DIR)/.dir:
	mkdir -p $(BUILD_DIR)
	touch $@

clean:
	rm -r $(BUILD_DIR)
