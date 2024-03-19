/** \file
 *  \brief Provides the private implementation of the Peripheral class
 *  
 * \ingroup systemc_example
*/
#include "peripheral.hpp"
#include "peripheral_register_interface.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>

#include <systemc>
using namespace sc_core;
#include <tlm.h>
using namespace tlm;
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
using namespace tlm_utils;


/** Private Implementation of the peripheral functionality. 
 * 
 *  Internal class content is just as an example and shouldn't be assumed would be similar
 *   between different peripheral models
 */
class Peripheral::Impl : public sc_module, private PeripheralRegisterInterface {
public:
    // Interfaces to outside world
    simple_initiator_socket_tagged<Impl> dma{"dma"};
    simple_target_socket_tagged<Impl>    register_if{"regif"};
    simple_target_socket_tagged<Impl>    memory_if{"memory"};
    sc_export<sc_fifo_in_if<uint8_t>>          fifo_out{"fifo_out"};
    sc_fifo_in<uint8_t>                        fifo_in{"fifo_in"};
    sc_export<sc_signal_in_if<bool>>           transfer_done_irq{"transfer_done_irq"};

    Impl(sc_module_name name = "impl") : sc_module(name) {
        // Bindings
        fifo_out.bind(fifo);
        transfer_done_irq.bind(irq_signal);
        register_if.register_b_transport(this, &Impl::b_transport, REGIF_TARGET_INDEX);
        dma.register_nb_transport_bw(this, &Impl::nb_transport_bw, 0);
        memory_if.register_b_transport(this, &Impl::b_transport, MEMORY_TARGET_INDEX);

        // Channel initialization
        irq_signal = false;

        // Processes
        SC_HAS_PROCESS(Impl);
        SC_THREAD(timer_done_thread); sensitive << timer_done_event; dont_initialize();
        SC_METHOD(fifo_data_received_method); sensitive << fifo_in.data_written(); dont_initialize();
    }

private:
    enum { REGIF_TARGET_INDEX, MEMORY_TARGET_INDEX };
    sc_fifo<uint8_t> fifo{"fifo"};
    sc_signal<bool> irq_signal{"irq_signal"};
    sc_time clock_period;
    sc_event timer_done_event{"timer_done_event"};

    void end_of_elaboration() override {
        // set clock period here, because time precision is guaranteed to be set during elaboration
        clock_period = sc_time(1.0, SC_NS);
    }

    void update_volatile_register_fields(sc_time & delay) {
        // Just update all fields, without checking which will be read
        // Consume delay, for simplicity
        wait(delay);
        delay = SC_ZERO_TIME;

        update_timer(delay);
    }

    void process_command_register_fields(sc_time & delay) {
        if (cmd_start_timer) {
            start_timer(delay);
            cmd_start_timer = false; // ack the command
        }
    }

    /** Timer is free running and can be started
     *   but never stopped.
     *  It is modeled without a clock in simulation
    */
    sc_time start_time;
    bool start_time_valid = false;
    void start_timer(sc_time & delay) {
        // Annotated delay is overly complicated here, but implementing just as an example
        auto new_start_time = sc_time_stamp() + delay;
        if (start_time_valid) {
            // At some point already or in the future another thread will start the timer
            // Adjust timer to be the earliest of the start times
            start_time = std::min(start_time, new_start_time);
        } else {
            start_time = new_start_time;
            start_time_valid = true;
        }
        
        // Schedule timer done notification at rollover time
        auto timer_done_time = start_time + (double(std::numeric_limits<decltype(timer)>::max()) + 1) * clock_period;
        auto current_time = sc_time_stamp();
        if (timer_done_time >= current_time) {
            timer_done_event.notify(timer_done_time - current_time);
        }
    }

    void update_timer(sc_time & delay) {
        // Keeping only one copy of timer and start_time* state variables, so maintaining annotated delay before
        //  updating timer is not supported
        wait(delay);
        delay = SC_ZERO_TIME;

        if (start_time_valid) {            
            auto timer_time = sc_time_stamp() - start_time;
            auto timer_cycles = timer_time / clock_period;

            // round up to even number of cycles and then annotate delay to account
            //  for any remainder time
            timer = std::ceil(timer_cycles);
            auto timer_cycles_rem = timer - timer_cycles;
            delay += timer_cycles_rem * clock_period;
        } // else timer never started so no need to update
    }

    void timer_done_thread() {
        timer_done = true;

        // Stream all data to fifo
        for(int i=0; i < memory.size(); ++i) {
            fifo.write(i);
        }

        return; // intentionally never will re-enter again
    }

    sc_time method_delay = SC_ZERO_TIME; // using method here and keeping all delays as annotated
    sc_time previous_time_stamp = SC_ZERO_TIME;
    unsigned memory_index = 0;
    unsigned memory_written_count = 0;

    /** Extremely simple memory manager, just used because required for nb_transport.
     *  No attempt to keep pool of transactions.
     *  Generic_payload handles all the reference count management, so this class really 
     *   just handles the freeing.
    */
    class SimpleMM : public tlm_mm_interface {
    public:
        auto & allocate_payload() { return *(new tlm_generic_payload(this)); }
        void free(tlm_generic_payload * payload) override {
            payload->reset();
            delete payload;
        }
        ~SimpleMM() = default;
    } simple_mm;

    void fifo_data_received_method() {
        // Adjust annotated delay as necessary for tracking

        while(fifo_in->num_available() > 0) {
            uint8_t value = fifo_in->read() ^ 0xFF;

            // For convenience, copy value into internal memory, so that data ptr will still be valid
            //  later on in non-blocking protocol, once the value is being read
            memory[memory_index] = value;

            // Could use a thread and invoke b_transport for simplicity,
            //  but instead using nb_transport for illustration
            auto & payload = simple_mm.allocate_payload();
            payload.acquire();
            payload.set_address(memory_index);
            payload.set_byte_enable_ptr(nullptr);
            payload.set_write();
            payload.set_data_length(sizeof(value));
            payload.set_data_ptr(&memory[memory_index++]);
            payload.set_streaming_width(payload.get_data_length());

            tlm_phase phase = BEGIN_REQ;

            // Target isn't necessarily handling delay, but let's limit our own interface
            //  to one new transfer per clock cycle, i.e., set temporal delay
            auto time_stamp = sc_time_stamp();
            auto delta = (time_stamp - previous_time_stamp);
            sc_time delay = SC_ZERO_TIME;
            if (delta < clock_period) {
                delay = clock_period - delta;
            }
            previous_time_stamp = time_stamp + delay;

            auto return_value = dma->nb_transport_fw(payload, phase, delay);

            if (return_value == TLM_COMPLETED) {
                assert(payload.is_response_ok());
                payload.release();
            } else if (phase == BEGIN_RESP) {
                assert(payload.is_response_ok());

                // Just finish the transfer
                phase = END_RESP;
                dma->nb_transport_fw(payload, phase, delay);
                assert(payload.is_response_ok());
                payload.release();
            }
        }
    }

    tlm_sync_enum nb_transport_bw(int ignored_id, tlm_generic_payload & payload, tlm_phase & phase, sc_time & delay) {
        payload.set_response_status(TLM_OK_RESPONSE);


        // Finish the transfer or just keep it going as necessary
        if (phase == END_REQ) {
            return TLM_ACCEPTED;
        }
        assert(phase == BEGIN_RESP);

        // Just finish the transfer
        phase = END_RESP;
        dma->nb_transport_fw(payload, phase, delay);
        payload.release();
        return TLM_UPDATED;
    }

    /** Implement single b_transport method for handling both target ports */
    void b_transport(int id, tlm_generic_payload & payload, sc_time & delay) {
        payload.set_dmi_allowed(false); // not supported for this peripheral (though could be added)

        // Compute address to be accessed within register interface of this peripheral
        void * base_ptr = nullptr;
        unsigned bus_width = 0;
        if (id == REGIF_TARGET_INDEX) {
            base_ptr = static_cast<PeripheralRegisterInterface*>(this);
            bus_width = register_if.get_bus_width() / 8;
        } else if (id == MEMORY_TARGET_INDEX) {
            base_ptr = &memory;
            bus_width = memory_if.get_bus_width() / 8;
        } else {
            assert(!"invalid index encountered");
        }
        auto offset = payload.get_address();
        auto periph_address = uintptr_t(base_ptr) + offset;

        auto data_length = payload.get_data_length();

        // Error checks (or unsupported features)
        if (payload.get_streaming_width() < data_length) {
            payload.set_response_status(tlm::tlm_response_status::TLM_BURST_ERROR_RESPONSE);
            return;
        }
        if (payload.get_byte_enable_ptr() != nullptr) {
            payload.set_response_status(tlm_response_status::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            return;
        }

        payload.set_response_status(TLM_OK_RESPONSE);

        switch(payload.get_command()) {
            case TLM_READ_COMMAND:
                // Handle any special pre-read actions
                update_volatile_register_fields(delay);

                std::memcpy(payload.get_data_ptr(), (void*)periph_address, data_length);

                // Record delay in simple manner
                delay += std::ceil(double(data_length) / bus_width) * clock_period;

                // No special post-read actions

                break;
            case TLM_WRITE_COMMAND:
                // No special pre-write actions

                // Consume any delay in case other threads are behind the annotated time and need to execute
                wait(delay); // consume delay before writing registers in case other threads would update in the meantime
                delay = SC_ZERO_TIME;

                std::memcpy((void*)periph_address, payload.get_data_ptr(), data_length);

                // Record delay in simple manner
                delay += std::ceil(double(data_length) / bus_width) * clock_period;

                // Handle any special post-write actions
                process_command_register_fields(delay);

                if (id == MEMORY_TARGET_INDEX) {
                    auto value = payload.get_data_ptr()[0];
                    memory_written_count += data_length;
                    if (memory_written_count == memory.size()) {
                        wait(delay); // consume delay before asserting irq
                        delay = SC_ZERO_TIME;
                        irq_signal = true;
                    }
                }

                break;
            case TLM_IGNORE_COMMAND:
                break;
            default:
                assert(!"unexpected command encountered");
        }
    }

    // // Requirements for pimpl
    // ~Impl() = default;

};


Peripheral::Peripheral(sc_module_name name) : sc_module(name), pimpl(new Impl("impl")) {
    // Bindings
    fifo_out.bind(pimpl->fifo_out);
    transfer_done_irq.bind(pimpl->transfer_done_irq);
    register_if.bind(pimpl->register_if);
    memory_if.bind(pimpl->memory_if);
    pimpl->dma.bind(dma);
    pimpl->fifo_in.bind(fifo_in);
}