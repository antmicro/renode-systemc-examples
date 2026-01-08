#include <systemc>
#include <tlm>
#include "timekeeper.h"

timekeeper::timekeeper(sc_core::sc_module_name name) {
  //
}

void timekeeper::b_transport(tlm::tlm_generic_payload& payload, sc_core::sc_time& time) {
  if (payload.is_read()) {
    payload.set_response_status(tlm::TLM_OK_RESPONSE);
    uint8_t *data = payload.get_data_ptr();
    *((uint64_t *)data) = (uint64_t)(sc_core::sc_time_stamp().to_seconds());
  }

  return;
}

bool timekeeper::get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&) {
  return false;
}

tlm::tlm_sync_enum timekeeper::nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&) {
  return tlm::TLM_COMPLETED;
}

unsigned int timekeeper::transport_dbg(tlm::tlm_generic_payload&) {
  return 0;
}
