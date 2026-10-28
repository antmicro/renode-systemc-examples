// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "bus.h"

SC_HAS_PROCESS(bus);
bus::bus(sc_core::sc_module_name name) : sc_module(name) {
  renode_target_socket.register_b_transport(this, &bus::b_transport);
}

void bus::b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay) {
  const auto address = payload.get_address();

  if (address < 0x1000) {
    control_initiator_socket->b_transport(payload, delay);
  } else {
    payload.set_address(address - 0x1000);
    memory_initiator_socket->b_transport(payload, delay);
  }
}
