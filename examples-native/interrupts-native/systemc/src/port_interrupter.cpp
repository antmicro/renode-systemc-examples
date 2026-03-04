#include <systemc>
#include <tlm>
#include <cstdint>
#include "port_interrupter.h"

port_interrupter::port_interrupter(sc_core::sc_module_name name, int irq_interval)
  : sc_core::sc_module(name),
    irq_interval(irq_interval) {
  SC_THREAD(interrupt_loop);
}

void port_interrupter::interrupt_loop() {
  while (true) {
    irq_out.write(true);
    wait(1, sc_core::SC_SEC);
  }
}

void port_interrupter::b_transport(tlm::tlm_generic_payload& payload, sc_core::sc_time& time) {
  if (payload.get_command() == tlm::TLM_WRITE_COMMAND) {
    irq_out.write(false);
  }
  return;
}

bool port_interrupter::get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&) {
  return false;
}

tlm::tlm_sync_enum port_interrupter::nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&) {
  return tlm::TLM_COMPLETED;
}

unsigned int port_interrupter::transport_dbg(tlm::tlm_generic_payload&) {
  return 0;
}
