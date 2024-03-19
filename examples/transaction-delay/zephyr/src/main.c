#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS   1000

int main(void)
{
    volatile uint32_t* systemc_seconds = (uint32_t*)0x9000000;

	while (1) {
		printf("SystemC virtual time (1s transaction delay): %d s\n", *systemc_seconds);
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
