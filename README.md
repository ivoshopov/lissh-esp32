
Running the interpreter on Espressif chip

	source ~/projects/esp/esp-idf/export.sh
	make
	cd build
	make ESPPORT=/dev/ttyUSB1 flash monitor


Debugging
	https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-guides/tools/idf-monitor.html#launching-gdb-with-gdbstub
