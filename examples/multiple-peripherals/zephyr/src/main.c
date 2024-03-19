/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define PERIPHERAL_A 0x7000000

#define PERIPHERAL_C_TRANSFER_DONE_IRQ 57
#define IRQ_PRIO 2 

typedef struct {
    uint8_t cmd_start_timer;
    uint32_t timer;
    uint8_t timer_done;
    uint8_t irq_ack;
    uint8_t memory[100];
} PeripheralCSR;

static uint8_t example_done = 0;

void interrupt_handler(void* arg) {
    printf("Interrupt handler invoked.\n");
    example_done = 1;

    irq_disable(PERIPHERAL_C_TRANSFER_DONE_IRQ);
}

int main(void)
{
    IRQ_CONNECT(PERIPHERAL_C_TRANSFER_DONE_IRQ, IRQ_PRIO, interrupt_handler, 0, 0);

    irq_enable(PERIPHERAL_C_TRANSFER_DONE_IRQ);

    volatile PeripheralCSR* peripheralA = (PeripheralCSR*)PERIPHERAL_A;

    peripheralA->cmd_start_timer = 1;

    k_msleep(1000);
    while (example_done == 0) {
        printf("Peripheral A timer: %u\n", peripheralA->timer);
        printf("Timer done: %u\n", peripheralA->timer_done);
        k_msleep(1000);
    }

    printf("Example complete!\n");

    return 0;
}
