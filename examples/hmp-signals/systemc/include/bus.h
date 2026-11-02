// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

class bus : public sc_core::sc_module {
public:
  bus(sc_core::sc_module_name name);

  tlm_utils::simple_target_socket<bus> renode_target_socket;

  tlm_utils::simple_initiator_socket<bus> control_initiator_socket;
  tlm_utils::simple_initiator_socket<bus> memory_initiator_socket;

private:
  void b_transport(tlm::tlm_generic_payload &payload,
                   sc_core::sc_time &delay);
};
