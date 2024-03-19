#include "timekeeper.h"
#include <iostream>

timekeeper::timekeeper(sc_core::sc_module_name name) {
  target_socket.register_b_transport(this, &timekeeper::custom_b_transport);
}

void timekeeper::custom_b_transport(tlm::tlm_generic_payload &payload,
                                    sc_core::sc_time &delay_time) {
  if (payload.is_read()) {
    payload.set_response_status(tlm::TLM_OK_RESPONSE);
    uint8_t *data = payload.get_data_ptr();
    *((uint64_t *)data) = (uint64_t)(sc_core::sc_time_stamp().to_seconds());
    delay_time = sc_core::sc_time(1, sc_core::SC_SEC);

    return;
  }

  payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
}
