#pragma once

#include <memory>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

struct bus : public sc_core::sc_module {
  bus(sc_core::sc_module_name name);

  tlm_utils::simple_target_socket<bus> renode_target_socket;

  tlm_utils::simple_initiator_socket<bus> initiator_socket0;
  tlm_utils::simple_initiator_socket<bus> initiator_socket1;

private:
  void on_renode_request(tlm::tlm_generic_payload &payload,
                         sc_core::sc_time &delay_time);
  void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                 sc_dt::uint64 end_range);
};
