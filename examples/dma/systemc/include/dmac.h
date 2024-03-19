#pragma once

#include <memory>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

// Models a very basic DMA controller.
// Registers:
// offset       function
// 0x4          source memory address (32-bit)
// 0x8          destination memory address (32-bit)
// 0xC          data length in bytes
// 0x10         control register
//              0x1: start memory transfer
class dmac : public sc_core::sc_module {
public:
  dmac(sc_core::sc_module_name name);

  // Socket for sending requests to the system bus.
  tlm_utils::simple_initiator_socket<dmac> bus_initiator_socket;

  // Socket for servicing requests from the system bus.
  tlm_utils::simple_target_socket<dmac> bus_target_socket;

  // Raised when DMAC requires bus access, lowered when access no longer needed.
  sc_core::sc_out<bool> bus_request;

  // Positive edge event signals that a pending memory operation can be
  // performed.
  sc_core::sc_in<bool> bus_free;

  void dmac_loop();

private:
  void receive_b_transport(tlm::tlm_generic_payload &trans,
                           sc_core::sc_time &delay);

  sc_core::sc_event start_event;
  uint32_t addr_src;
  uint32_t addr_dst;
  uint32_t data_len;

  std::unique_ptr<tlm::tlm_generic_payload> payload;
};
