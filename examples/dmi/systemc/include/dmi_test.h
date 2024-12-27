#pragma once

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

// Dependency on renode_bridge is only necessary to call
// "invalidate_translation_blocks".
// We use direct reference here to keep it simple; in practice one would likely
// use this interface indirectly through standard SystemC mechanisms.
#include "renode_bridge.h"

#include <memory>

class dmi_test : public sc_core::sc_module {
public:
  dmi_test(sc_core::sc_module_name name, renode_bridge &renode_bridge);

  // Socket for sending requests to the system bus
  tlm_utils::simple_initiator_socket<dmi_test> bus_initiator_socket;

  // Positive edge event signals used to start the test sequence
  sc_core::sc_in<bool> test_begin;

  void dmi_test_sequence();

private:
  std::unique_ptr<tlm::tlm_generic_payload> payload;

  renode_bridge &m_renode_bridge;
};
