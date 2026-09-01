// Copyright (c) 2026 Antmicro <www.antmicro.com>
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "renode_bridge.h"
#include "librenode.h"

class trivial_target : public sc_core::sc_module {
public:
  explicit trivial_target(sc_core::sc_module_name name) : sc_module(name), socket("socket") {
    socket.register_b_transport(this, &trivial_target::b_transport);
    socket.register_get_direct_mem_ptr(this, &trivial_target::get_direct_mem_ptr);
  }

  bool failed() const {
    return workload_failed;
  }

  tlm_utils::simple_target_socket<trivial_target> socket;

private:
  void b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &) {
    payload.set_dmi_allowed(false);

    if(payload.get_data_length() != sizeof(std::uint32_t) || payload.get_address() >= Size) {
      payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      return;
    }

    if(payload.is_read()) {
      std::uint32_t value = 0;
      std::memcpy(payload.get_data_ptr(), &value, sizeof(value));
    } else if(payload.is_write()) {
      if(payload.get_address() == CompletionOffset) {
        std::uint32_t value;
        std::memcpy(&value, payload.get_data_ptr(), sizeof(value));
        workload_failed = value != 0;
        sc_core::sc_stop();
      }
    } else {
      payload.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      return;
    }

    payload.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  bool get_direct_mem_ptr(tlm::tlm_generic_payload &, tlm::tlm_dmi &) {
    return false;
  }

  static constexpr std::uint64_t Size = 0x4000;
  static constexpr std::uint64_t CompletionOffset = Size - sizeof(std::uint32_t);
  bool workload_failed = false;
};

static bool renode_command(const std::string &command) {
  char output[4096] = {};
  char error[4096] = {};
  const auto status = renode_exec_command_ex(command.c_str(), output, sizeof(output), error, sizeof(error));
  if(status == RENODE_SUCCESS) {
    return true;
  }
  std::cerr << "Renode command failed (" << status << "): " << command << std::endl;
  if(*output) {
    std::cerr << output;
  }
  if(*error) {
    std::cerr << error;
  }
  return false;
}

int sc_main(int argc, char **argv) {
  if(argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <resc> <iterations> <peripherals>" << std::endl;
    return 2;
  }

  if(renode_init(nullptr, -1, -1) != RENODE_SUCCESS ||
     !renode_command(std::string("$iterations=") + argv[2]) ||
     !renode_command(std::string("$peripherals=") + argv[3]) ||
     !renode_command(std::string("include @") + argv[1])) {
    return 2;
  }

  trivial_target target("target");
  renode_bridge bridge("bridge", "", "", true, "peripheral-profile", "sysbus.systemc");
  bridge.initiator_socket.bind(target.socket);

  if(!renode_command("start")) {
    return 2;
  }
  sc_core::sc_start();
  renode_exec_command("quit");

  return target.failed() ? 1 : 0;
}
