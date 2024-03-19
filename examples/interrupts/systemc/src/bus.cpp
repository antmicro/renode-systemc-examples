#include "bus.h"
#include <stdio.h>

SC_HAS_PROCESS(bus);
bus::bus(sc_core::sc_module_name name) {
  renode_target_socket.register_b_transport(this, &bus::on_renode_request);
}

void bus::invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                    sc_dt::uint64 end_range) {
  fprintf(stderr,
          "[ERROR] invalidate_direct_mem_ptr not implemented for bus.\n");
}

void bus::on_renode_request(tlm::tlm_generic_payload &payload,
                            sc_core::sc_time &delay_time) {
  if (payload.get_command() == tlm::TLM_WRITE_COMMAND) {
    uint64_t address = payload.get_address();
    if (address == 0) {
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
      initiator_socket0->b_transport(payload, delay);
      delay_time = delay;
      payload.set_response_status(tlm::TLM_OK_RESPONSE);
      return;
    } else if ((address & 0x100000) != 0) {
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
      initiator_socket1->b_transport(payload, delay);
      delay_time = delay;
      payload.set_response_status(tlm::TLM_OK_RESPONSE);
      return;
    }
  }

  payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
}
