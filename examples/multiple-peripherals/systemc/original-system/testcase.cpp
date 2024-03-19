/** \file 
 *  Contains test software which configures the peripherals and checks execution progress and final results via the system bus interface.
 * 
 * \ingroup systemc_example
*/

#include <cassert>

#include "platform_sw_api.hpp"

/** This testcase runs real embedded software on the virtual platform.
 *  Any interactions with model environment is handled by the platform simulator
 *   so that same source code can compile on any platform whether virtual or real,
 *   and with different CPU types or modeling abstractions.
 * 
 *  This is the main thread execution steps:
 *    - Start peripheral 0 timer
 *    - On triggered timer overflow, peripheral 0 writes to peripheral 1
 *    - After finished writing, peripheral 0 streams data to peripheral 1 through side-channel
 *    - Peripheral 1, upon receiving data, modifies it and streams it to memory (periph 2) via main bus
 *    - Peripheral 2 asserts an IRQ once it has received all data
 *    - CPU operates in 3 modes:  polling until half-done, sleeping with WFI, and interrupt handler thread
 *      - polls until timer is mostly done, checks status is still incomplete
 *      - starts sleeping and waits for interrupt. when awakes, checks is now complete
 *      - IRQ handler thread checks that data was copied to memory correctly
*/
void testcase() {
    WRITE_PERIPHERAL_FIELD(0, cmd_start_timer, true);
    
    uint64_t timer_value;
    while((timer_value = READ_PERIPHERAL_FIELD(0, timer)) < 1000) {
        assert(not READ_PERIPHERAL_FIELD(0, timer_done));
    }

    cpu_wfi();
    assert(READ_PERIPHERAL_FIELD(0, timer_done));
    while(not READ_PERIPHERAL_FIELD(2, irq_ack)); // make sure irq handler thread finished
}


/** Simple model of a single IRQ handler thread that is invoked upon rising edge of IRQ */
void irq_handler() {
    // Check that data was written to memory as expected
    auto read_memory = READ_PERIPHERAL_FIELD(2, memory);
    for(int i=0; i < read_memory.size(); ++i) {
        auto value = read_memory[i];
        assert(value == (i ^ 0xFF));
    }

    WRITE_PERIPHERAL_FIELD(2, irq_ack, true);
}
