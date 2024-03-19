/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define IRQ_SOCKET_INTERRUPTER 55
#define IRQ_PORT_INTERRUPTER 56
#define IRQ_PRIO 1

#define SYSTEMC_INTERRUPTER0 0x9000000
#define SYSTEMC_INTERRUPTER1 0x9100000

void interrupt_handler_socket(void* arg) {
    volatile uint32_t* peripheral = (uint32_t*)SYSTEMC_INTERRUPTER0;
    printf("Interrupt handler for interrupter 0 (every 1 second)\n");
    *peripheral = 1;
}

void interrupt_handler_port(void* arg) {
    volatile uint32_t* peripheral = (uint32_t*)SYSTEMC_INTERRUPTER1;
    printf("Interrupt handler for interrupter 1 (every 3 seconds)\n");
    *peripheral = 1;
}

int main(void)
{
    IRQ_CONNECT(IRQ_SOCKET_INTERRUPTER, IRQ_PRIO, interrupt_handler_socket, 0, 0);
    IRQ_CONNECT(IRQ_PORT_INTERRUPTER, IRQ_PRIO, interrupt_handler_port, 0, 0);

    irq_enable(IRQ_SOCKET_INTERRUPTER);
    irq_enable(IRQ_PORT_INTERRUPTER);

	while (1) {
		printf("Zephyr main thread: Zzzz\n");
	    k_msleep(10000);
	}

	return 0;
}
