#pragma once

#include <memory>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

// Attempts to write to addr_dst on Renode sysbus every 1 second of SystemC
// virtual time
class writer: public sc_core::sc_module {
public:
  writer(sc_core::sc_module_name name, uint32_t addr_dst);

  tlm_utils::simple_initiator_socket<writer> bus_initiator_socket;

  void write_loop();

private:
  uint32_t write_address;
  std::unique_ptr<tlm::tlm_generic_payload> payload;
};
