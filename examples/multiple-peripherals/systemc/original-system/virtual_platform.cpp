/**
 * Copyright (c) 2024 Antmicro
 * Copyright (c) 2024 Silicon Labs
*/

/** \file
 *  \brief Contains a virtual platform for simulating a CPU connected to peripherals and executing code
 * 
 * \ingroup systemc_example
*/
#include "peripheral.hpp"
#include "peripheral_register_interface.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>

#include <systemc>
using namespace sc_core;
#include <tlm.h>
using namespace tlm;
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
using namespace tlm_utils;


void testcase(); // forward declaration of testcase to be linked in separately (see Cpu::main_thread)
void irq_handler(); // forward declaration of IRQ handler to be linked in separately (see Cpu::main_thread)


// Simple CPU model with one thread representing the main function
class Cpu : public sc_module {
    using uintptr_t = std::uintptr_t;
public:
    // Interfaces to outside modules
    multi_passthrough_initiator_socket<Cpu> systembus{"systembus"};
    sc_in<bool> irq;

    // Standard SystemC constructor
    Cpu(sc_module_name name = "cpu") : sc_module(name) {
        SC_HAS_PROCESS(Cpu);
        SC_THREAD(main_thread);
        SC_THREAD(irq_handler_thread); sensitive << irq.pos(); dont_initialize();

        // Set up address decoding table (see systembus_memcpy)
        static_assert(sizeof(PeripheralRegisterInterface) <= 0x1000); // Ensure fits into allotted region size hardcode below
        peripheral_ranges.push_back(std::make_pair(0x0000, 0x0FFF));
        peripheral_ranges.push_back(std::make_pair(0x1000, 0x1FFF));
        peripheral_ranges.push_back(std::make_pair(0x2000, 0x2FFF));
    }

private:
    std::vector<std::pair<uintptr_t, uintptr_t>> peripheral_ranges;

    void end_of_elaboration() override {
        assert(systembus.bind_count() == peripheral_ranges.size());
    }

    void systembus_transfer(tlm::tlm_command command, uintptr_t address, unsigned char *data_ptr, std::size_t count)
    {
        tlm_generic_payload payload;
        payload.set_command(command);
        payload.set_data_ptr(data_ptr);
        payload.set_data_length(count);
        payload.set_streaming_width(payload.get_data_length()); // not a streaming transfer

        // Generally a system bus would decode target addresses and route to correct
        //  target, but for simplicity, just doing decoding here and using a multi-socket
        //  to bound CPU to all targets directly
        int target_index = -1;
        bool mapped = false;
        for (auto &peripheral_range : peripheral_ranges)
        {
            target_index++;
            if (peripheral_range.first <= address and address <= peripheral_range.second)
            {
                mapped = true;
                address -= peripheral_range.first;  // Remove bus offset before passing address to peripheral
                break;
            }
        }

        payload.set_address(address);

        sc_time delay = SC_ZERO_TIME;
        systembus[target_index]->b_transport(payload, delay);
        assert(payload.is_response_ok() or !"Response not okay as expected");
        wait(delay);
        delay = SC_ZERO_TIME;
    }
    /** Main thread for execution. In general, this would invoke a callback based on the specific test,
     *   but for simplicity, just invoking a function called testcase() that must be linked in
    */
    void main_thread() {
        testcase();

        stop();
    }

    void stop() {
        #ifdef EMBEDDED
            exit();
        #endif

        SC_REPORT_INFO(name(), "Test complete. Exiting main thread");
        sc_stop();
    }

    /** Simple model of a single IRQ handler thread that is invoked upon rising edge of IRQ */
    void irq_handler_thread() {
        assert(irq.posedge());

        irq_handler();

        return; // purposely just executing thread once ever in this case
    }

public: // Following are public just for convenience so don't have to define API accessors as friend functions

    /** Rudimentary CPU modeling of wait-for-interrupt function.
     *  Check if IRQ is asserted, and if not, sleep until it is
    */
    void wfi() {
        #ifdef EMBEDDED
          WFI();
          return;
        #endif

        if (not irq) wait(irq.posedge_event());
    }

    /** Convenience function for code to get access to peripheral base address by index.
     *  In real code, this would be made constexpr vs. dynamic looking up at run-time
     */
    auto get_peripheral_address(int index) {
        auto base_address = peripheral_ranges[index].first;
        assert(base_address % alignof(PeripheralRegisterInterface) == 0 or !"alignment doesn't match");
        return base_address;
    }

    // Virtual platform support functions for accessing memory or peripherals over the system bus
    // Embedded platforms would just inline the memcpy directly, but here
    //  since using simple CPU model to execute the code, have to map the 
    //  accesses to TLM directly (Renode would need something like this to handle bus accesses)
    uintptr_t systembus_memcpy_write(uintptr_t dest, const void * src, std::size_t count) {
        #ifdef EMBEDDED
            return std::memcpy((void*)dest, src, count);
        #endif

        auto command = TLM_WRITE_COMMAND;
        auto data_ptr = static_cast<unsigned char *>(const_cast<void *>(src));
        systembus_transfer(command, dest, data_ptr, count);

        return dest;
    }

    void * systembus_memcpy_read(void * dest, uintptr_t src, std::size_t count) {
        #ifdef EMBEDDED
            return std::memcpy(dest, src, count);
        #endif

        auto command = TLM_READ_COMMAND;
        auto data_ptr = static_cast<unsigned char *>(const_cast<void *>(dest));
        systembus_transfer(command, src, data_ptr, count);

        return dest;
    }
};

class Top : public sc_module {
public:
    // Standard SystemC constructor
    Top(sc_module_name name = "top") : sc_module(name) {
        // Bindings of signals across peripherals
        // For convenience, just binding together in a uniform manner, even if
        //  some will be idle
        for (int i=0; i < peripheral.size(); ++i) {
            cpu.systembus.bind(peripheral[i].register_if);
            peripheral[i].dma.bind(peripheral[(i+1) % peripheral.size()].memory_if);
            // peripheral[i].fifo_out.bind(peripheral[(i+1) % peripheral.size()].fifo_in);
            peripheral[(i+1) % peripheral.size()].fifo_in.bind(peripheral[i].fifo_out);
        }
        cpu.irq.bind(peripheral[2].transfer_done_irq); // just connect up one IRQ b/c CPU model doesn't have vectored IRQ support
    }

private:
    // Sub-modules
    Cpu cpu;
    sc_vector<Peripheral> peripheral{"peripheral", 3};
};


int sc_main( int argc, char* argv[] ) {
    Top top;

    sc_start();  
    // Detect abnormal paused condition, e.g., from starvation
    if(sc_get_status() == SC_PAUSED) {
        SC_REPORT_ERROR("sc_main", "Abnormal starvation condition encountered");
        sc_stop();
    }
    assert(sc_get_status() == SC_STOPPED or !"Mismatch with expected SC simulation phase");

    return 0;
}


/** Helper function used by platform API definitions below
 * 
 *  For convenience, just assume there is one cpu and 
*/
// auto & get_current_cpu() {
//     auto * cpu_obj = sc_find_object("top.cpu");
//     assert(cpu_obj or !"'cpu' not found");
//     return *dynamic_cast<Cpu*>(cpu_obj);
// }    



auto & get_current_cpu() {
    auto handle = sc_get_current_process_handle();
    sc_object * obj = handle.get_parent_object();
    return dynamic_cast<Cpu &>(*obj);
}


// Provide platform API definitions
#include "platform_sw_api.hpp"

uintptr_t systembus_memcpy_write(uintptr_t dest, const void * src, std::size_t count) {
    return get_current_cpu().systembus_memcpy_write(dest, src, count);
}
void * systembus_memcpy_read(void * dest, uintptr_t src, std::size_t count) {
    return get_current_cpu().systembus_memcpy_read(dest, src, count);
}
void cpu_wfi() {
    get_current_cpu().wfi();
}
uintptr_t get_peripheral_address(int index) {
    return get_current_cpu().get_peripheral_address(index);
}
