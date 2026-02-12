#pragma once

#include <systemc>
#include <tlm>

struct timekeeper : sc_core::sc_module, tlm::tlm_fw_transport_if<> {
  timekeeper(sc_core::sc_module_name name);

  void b_transport(tlm::tlm_generic_payload&, sc_core::sc_time&) override;
  bool get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&) override;
  tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&) override;
  unsigned int transport_dbg(tlm::tlm_generic_payload&) override;
};
