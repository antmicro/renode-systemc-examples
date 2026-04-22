// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "timekeeper.h"
#include <iostream>
#include <assert.h>

timekeeper::timekeeper(sc_core::sc_module_name name) {
  target_socket.register_b_transport(this, &timekeeper::custom_b_transport);
}

void timekeeper::custom_b_transport(tlm::tlm_generic_payload &payload,
                                    sc_core::sc_time &delay_time) {
  if (payload.is_read()) {
    // detects mismatch if the tested Zephyr binary changes
    // and alters the expected uint32_t reads
    assert(payload.get_data_length() == sizeof(std::uint32_t));
    payload.set_response_status(tlm::TLM_OK_RESPONSE);
    uint8_t *data = payload.get_data_ptr();
    *((uint32_t *)data) = (uint32_t)(sc_core::sc_time_stamp().to_seconds());

    return;
  }

  payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
}
