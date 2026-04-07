// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <systemc>
#include <tlm>
#include <cstdint>
#include "gpio_peripheral.h"

gpio_peripheral::gpio_peripheral(sc_core::sc_module_name name) : sc_core::sc_module(name) {
  SC_METHOD(gpio_changed);
  sensitive << gpio_in;
}

void gpio_peripheral::b_transport(tlm::tlm_generic_payload& payload, sc_core::sc_time& time) {
  if (!payload.is_read())
    return;

  std::uint8_t ret = gpio_in.read();
  std::memcpy(payload.get_data_ptr(), &ret, sizeof(ret));
}

bool gpio_peripheral::get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&) {
  return false;
}

tlm::tlm_sync_enum gpio_peripheral::nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&) {
  return tlm::TLM_COMPLETED;
}

unsigned int gpio_peripheral::transport_dbg(tlm::tlm_generic_payload&) {
  return 0;
}

void gpio_peripheral::gpio_changed() {
  std::cout << "GPIO signal receiver in SystemC" << std::endl;
}
