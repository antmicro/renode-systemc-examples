#pragma once

#include <memory>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

struct port_interrupter : public sc_core::sc_module {
  port_interrupter(sc_core::sc_module_name name, uint32_t irq_interval_s);

  tlm_utils::simple_target_socket<port_interrupter> target_socket;

  sc_core::sc_out<bool> irq_out;

private:
  void interrupt_loop();

  void on_renode_request(tlm::tlm_generic_payload &payload,
                         sc_core::sc_time &delay_time);

  uint8_t data[8];
  uint32_t irq_interval_s;
};
