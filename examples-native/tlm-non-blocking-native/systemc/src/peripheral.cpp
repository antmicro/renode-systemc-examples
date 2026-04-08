#include <systemc>
#include <tlm>
#include <cstdint>
#include "peripheral.h"

peripheral::peripheral(sc_core::sc_module_name name) : sc_core::sc_module(name), initiator(nullptr), m_peq("peq") {
  SC_THREAD(peq_thread);
}

void peripheral::peq_thread() {
  while (true) {
    wait(m_peq.get_event());

    while (auto *trans = m_peq.get_next_transaction()) {
      auto cmd = trans->get_command();
      auto *ptr = trans->get_data_ptr();

      if (trans->is_write()) {
        reg_value = *reinterpret_cast<uint32_t*>(trans->get_data_ptr());
      } else if (trans->is_read()) {
        std::memcpy(trans->get_data_ptr(), &reg_value, sizeof(reg_value));
      };

      if (!initiator) {
        return;
      }

      tlm::tlm_phase phase = tlm::BEGIN_RESP;
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
      trans->set_response_status(tlm::TLM_OK_RESPONSE);
      initiator->nb_transport_bw(*trans, phase, delay);
    }
  }
}

void peripheral::b_transport(tlm::tlm_generic_payload& payload, sc_core::sc_time&) {
  if (payload.is_read()) {
    std::memcpy(payload.get_data_ptr(), &reg_value, sizeof(reg_value));
  } else if (payload.is_write()) {
    reg_value = *reinterpret_cast<uint32_t*>(payload.get_data_ptr());
  }
}

bool peripheral::get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&) {
  return false;
}

tlm::tlm_sync_enum peripheral::nb_transport_fw(tlm::tlm_generic_payload& payload, tlm::tlm_phase& phase, sc_core::sc_time& time) {
  switch (phase) {
  case tlm::BEGIN_REQ:
    m_peq.notify(payload, time);
    phase = tlm::END_REQ;
    return tlm::TLM_UPDATED;
  case tlm::END_RESP:
    return tlm::TLM_COMPLETED;
  default:
    return tlm::TLM_ACCEPTED;
  }
}

unsigned int peripheral::transport_dbg(tlm::tlm_generic_payload&) {
  return 0;
}
