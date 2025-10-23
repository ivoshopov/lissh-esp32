#include <stdio.h>
#include <io.h>
#include <module/module.h>
#include "driver/uart.h"


static int uartx_write(uart_port_t uart_num, const char c) {
        if (uart_write_bytes(uart_num, &c, 1) < 0)
                /* TODO: we need user readable error code, -1 isn't understandable */
                return -1;
        return 0;
}

static int uartx_read(uart_port_t uart_num) {
        uint8_t data;
        int read = uart_read_bytes(uart_num, &data, 1, portMAX_DELAY);
        if (read < 0) {
                return -1;
        }
        return data;
}
static int uart0_write(struct io_primitive *port, const char c) {
        return uartx_write(UART_NUM_0, c);
}

static int uart1_write(struct io_primitive *port, const char c) {
        return uartx_write(UART_NUM_1, c);
}

static int uart2_write(struct io_primitive *port, const char c) {
        return uartx_write(UART_NUM_2, c);
}

static int uart0_read(struct io_primitive *port) {
        return uartx_read(UART_NUM_0);
}

static int uart1_read(struct io_primitive *port) {
        return uartx_read(UART_NUM_1);
}

static int uart2_read(struct io_primitive *port) {
        return uartx_read(UART_NUM_2);
}

static struct io_primitive uart0_hndlr = {
        .read = uart0_read,
        .write = uart0_write,
};

static struct io_primitive uart1_hndlr = {
        .read = uart1_read,
        .write = uart1_write,
};

static struct io_primitive uart2_hndlr = {
        .read = uart2_read,
        .write = uart2_write,
};

lexp stream_write(struct io_primitive *port, lexp);
lexp stream_read(struct io_primitive *port);

static lexp uart_write(struct io_typ *port, lexp exp) {
        struct io_primitive *port_primitive = port->private;
        exp = stream_write(port_primitive, exp);
        port_primitive->write(port_primitive, '\n');
        port_primitive->write(port_primitive, '\r');
        return exp;
}

static lexp uart_read(struct io_typ *port) {
        struct io_primitive *port_primitive = port->private;
        return stream_read(port_primitive);
}

PORTS_SECTION struct io_typ uart0_port = {
        .private = &uart0_hndlr,
        .read = uart_read,
        .write = uart_write,
        .proto = "uart0",
};


PORTS_SECTION struct io_typ uart1_port = {
        .private = &uart1_hndlr,
        .read = uart_read,
        .write = uart_write,
        .proto = "uart1",
};


PORTS_SECTION struct io_typ uart2_port = {
        .private = &uart2_hndlr,
        .read = uart_read,
        .write = uart_write,
        .proto = "uart2",
};



static void init() {
        uart_config_t uart_config = {
                .baud_rate = 115200,
                .data_bits = UART_DATA_8_BITS,
                .parity    = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .source_clk = UART_SCLK_DEFAULT,
        };

#if !defined(CONFIG_UART1_AS_STDIO)
        if (uart_param_config(UART_NUM_1, &uart_config) != ESP_OK) {
                printf("UART driver configuration failed\n");
                fflush(stdout);
                return;
        }

        /* Tx: 23      (GPIO23, VSPID, HS1_STROBE)
           Rx: 22      (GPIO22, VSPIWP, U0RTS, EMAC_TXD1) */
        if (uart_set_pin(UART_NUM_1, 23, 22, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
                //if (uart_set_pin(UART_NUM_1, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
                printf("UART pin setup failed\n");
                fflush(stdout);
                return;
        }

        if (uart_driver_install(UART_NUM_1, 2 * 1024, 0, 0, NULL, 0) != ESP_OK) {
                printf("UART driver installation failed\n");
                fflush(stdout);
                return;
        }
#endif /* !defined(CONFIG_UART1_AS_STDIO) */


        /* UART2 */
        if (uart_param_config(UART_NUM_2, &uart_config) != ESP_OK) {
                printf("UART driver configuration failed\n");
                fflush(stdout);
                return;
        }

        /* Tx: 17       (GPIO17, HS1_DATA5, U2TXD, EMAC_CLK_OUT_180)
           Rx: 16       (GPIO16, HS1_DATA4, U2RXD, EMAC_CLK_OUT) */
        if (uart_set_pin(UART_NUM_2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
                //if (uart_set_pin(UART_NUM_1, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
                printf("UART pin setup failed\n");
                fflush(stdout);
                return;
        }

        if (uart_driver_install(UART_NUM_2, 2 * 1024, 0, 0, NULL, 0) != ESP_OK) {
                printf("UART driver installation failed\n");
                fflush(stdout);
                return;
        }
}

MODULE_SECTION struct module io_uart_module = {
        .setup = init,
};

