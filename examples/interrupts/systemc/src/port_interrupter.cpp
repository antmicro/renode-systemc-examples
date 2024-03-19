#include "port_interrupter.h"
#include <stdio.h>

SC_HAS_PROCESS(port_interrupter);
port_interrupter::port_interrupter(sc_core::sc_module_name name,
                                   uint32_t irq_interval_s)
    : irq_out("irq_out"), irq_interval_s(irq_interval_s) {
  SC_THREAD(interrupt_loop);

  irq_out.initialize(false);
  target_socket.register_b_transport(this,
                                     &port_interrupter::on_renode_request);
}

void port_interrupter::interrupt_loop() {
  while (true) {
    irq_out->write(true);
    wait(irq_interval_s, sc_core::SC_SEC);
  }
}

void port_interrupter::on_renode_request(tlm::tlm_generic_payload &payload,
                                         sc_core::sc_time &delay_time) {
  if (payload.get_command() == tlm::TLM_WRITE_COMMAND) {
    irq_out->write(false);
    payload.set_response_status(tlm::TLM_OK_RESPONSE);
  } else {
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
  }
}
