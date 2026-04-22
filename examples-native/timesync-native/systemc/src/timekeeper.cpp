// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <systemc>
#include <tlm>
#include <cstdint>
#include <assert.h>
#include "timekeeper.h"

timekeeper::timekeeper(sc_core::sc_module_name name) {
  // intentionally empty
}

void timekeeper::b_transport(tlm::tlm_generic_payload& payload, sc_core::sc_time& time) {
  if (!payload.is_read())
    return;

  // detects mismatch if the tested Zephyr binary changes
  // and alters the expected uint32_t reads
  assert(payload.get_data_length() == sizeof(std::uint32_t));
  auto ts = sc_core::sc_time_stamp().to_seconds();
  auto t = static_cast<std::uint32_t>(ts);

  std::memcpy(payload.get_data_ptr(), &t, sizeof(t));
}

bool timekeeper::get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&) {
  return false;
}

tlm::tlm_sync_enum timekeeper::nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&) {
  return tlm::TLM_COMPLETED;
}

unsigned int timekeeper::transport_dbg(tlm::tlm_generic_payload&) {
  return 0;
}
