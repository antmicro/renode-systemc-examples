#pragma once

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

struct timekeeper : public sc_core::sc_module {
  timekeeper(sc_core::sc_module_name name);

  tlm_utils::simple_target_socket<timekeeper> target_socket;

private:
  void custom_b_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay_time);
};
