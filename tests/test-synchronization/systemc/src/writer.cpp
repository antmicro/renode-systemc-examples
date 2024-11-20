#include "writer.h"

#include "tlm.h"

#include <stdio.h>

SC_HAS_PROCESS(writer);
writer::writer(sc_core::sc_module_name name, uint32_t write_address) :
  write_address(write_address) {
  SC_THREAD(write_loop);

  payload.reset(new tlm::tlm_generic_payload());
}

static void reset_payload(tlm::tlm_generic_payload &payload) {
  // Reset the TLM payload to reuse it between requests.
  payload.set_command(tlm::TLM_IGNORE_COMMAND);
  payload.set_address(0);
  payload.set_data_ptr(0);
  payload.set_data_length(1);
  payload.set_streaming_width(1);
  payload.set_byte_enable_ptr(nullptr);
  payload.set_byte_enable_length(0);
  payload.set_dmi_allowed(false);
  payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
}

void writer::write_loop() {
  uint8_t data;
  while (true) {
    sc_core::wait(1, sc_core::SC_SEC);

    reset_payload(*payload);
    payload->set_command(tlm::TLM_WRITE_COMMAND);
    payload->set_address(write_address);
    data = 0xab;
    payload->set_data_ptr(&data);

    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    bus_initiator_socket->b_transport(*payload, delay);
    if (payload->get_response_status() != tlm::TLM_OK_RESPONSE) {
       fprintf(stderr, "Failed to write to system bus\n");
       assert(false);
       return;
    }

    printf("writer: write complete\n");
  }
}
