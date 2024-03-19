#include "fifo_adapter.h"

SC_HAS_PROCESS(fifo_adapter);
fifo_adapter::fifo_adapter(sc_core::sc_module_name name): sc_module(name) {
    SC_METHOD(forward_fifo);
    sensitive << fifo_in.data_written();
    dont_initialize();

    fifo_target.register_b_transport(this, &fifo_adapter::on_fifo_in);

    fifo_out.bind(fifo);

    payload.reset(new tlm::tlm_generic_payload());
}

void fifo_adapter::forward_fifo() {
    uint8_t data = 0;
    while(fifo_in.nb_read(data)) {
        payload->set_command(tlm::TLM_WRITE_COMMAND);
        payload->set_address(0);
        payload->set_data_ptr(&data);
        payload->set_data_length(1);
        payload->set_streaming_width(1);
        payload->set_byte_enable_ptr(nullptr);
        payload->set_byte_enable_length(0);
        payload->set_dmi_allowed(false);
        payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        fifo_initiator->b_transport(*payload, delay);
    }
}

void fifo_adapter::on_fifo_in(tlm::tlm_generic_payload &payload,
                                         sc_core::sc_time &delay_time) {
    uint8_t value = *((uint8_t*)payload.get_data_ptr());
    fifo.write(value);

    payload.set_response_status(tlm::TLM_OK_RESPONSE);
}
