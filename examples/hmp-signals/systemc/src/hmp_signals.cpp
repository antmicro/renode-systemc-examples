// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "hmp_signals.h"

#include <cstring>
#include <iostream>

SC_HAS_PROCESS(hmp_signals);
hmp_signals::hmp_signals(sc_core::sc_module_name name)
    : sc_module(name), boot_address_value(DEFAULT_BOOT_ADDRESS),
      vtor_value(DEFAULT_VTOR) {
  boot_address.initialize(boot_address_value);
  vector_table_offset.initialize(vtor_value);
  cpu_wait.initialize(false);

  for (int core = 0; core < NUM_CORES; ++core) {
    reset_in[core].initialize(false);
    nmi_in[core].initialize(false);
    clock_period_ns[core] = DEFAULT_CLOCK_PERIOD_NS[core];
    clock_period[core].initialize(sc_core::sc_time(clock_period_ns[core], sc_core::SC_NS));
    for (auto &interrupt_input : interrupt_inputs[core]) {
      interrupt_input.initialize(false);
    }
    std::memcpy(&control_registers[core][CLOCK_PERIOD_OFFSET], &clock_period_ns[core], sizeof(clock_period_ns[core]));
    control_target_socket[core].register_b_transport(this, &hmp_signals::handle_control, core);
    memory_target_socket[core].register_b_transport(this, &hmp_signals::handle_memory, core);
  }
  std::memcpy(&control_registers[RiscV][BOOT_ADDRESS_OFFSET], &boot_address_value, sizeof(boot_address_value));
  std::memcpy(&control_registers[Arm][BOOT_ADDRESS_OFFSET], &vtor_value, sizeof(vtor_value));

  SC_THREAD(system_bus_writer);
}

void hmp_signals::handle_control(int core, tlm::tlm_generic_payload &payload, sc_core::sc_time &) {
  // Settle initial signals and preceding GPIO updates before reading them.
  wait(sc_core::SC_ZERO_TIME);
  const auto offset = payload.get_address();
  const auto length = payload.get_data_length();

  if (offset > control_registers[core].size() || length > control_registers[core].size() - offset) {
    payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  if (payload.is_read()) {
    if (core == Arm) {
      control_registers[core][SYSTEM_RESET_REQUEST_OFFSET] = system_reset_request.read();
    }
    std::memcpy(payload.get_data_ptr(), &control_registers[core][offset], length);
  } else if (payload.is_write()) {
    std::memcpy(&control_registers[core][offset], payload.get_data_ptr(), length);
    apply_control_write(core, offset, length);
  } else {
    payload.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return;
  }

  payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

void hmp_signals::handle_memory(int core, tlm::tlm_generic_payload &payload, sc_core::sc_time &) {
  const auto offset = payload.get_address();
  const auto length = payload.get_data_length();

  if (offset > memory[core].size() || length > memory[core].size() - offset) {
    payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  if (payload.is_read()) {
    std::memcpy(payload.get_data_ptr(), &memory[core][offset], length);
  } else if (payload.is_write()) {
    std::memcpy(&memory[core][offset], payload.get_data_ptr(), length);
  } else {
    payload.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return;
  }

  payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

void hmp_signals::apply_control_write(int core, uint64_t offset, unsigned length) {
  const auto overlaps = [offset, length](uint64_t start, uint64_t size) {
    return offset < start + size && start < offset + length;
  };

  for (unsigned i = 0; i < NUM_INTERRUPTS; ++i) {
    if (overlaps(i, 1)) {
      interrupt_inputs[core][i].write(control_registers[core][i] != 0);
    }
  }

  if (overlaps(RESET_OFFSET, 1)) {
    reset_in[core].write(control_registers[core][RESET_OFFSET] != 0);
  }
  if (overlaps(NMI_OFFSET, 1)) {
    nmi_in[core].write(control_registers[core][NMI_OFFSET] != 0);
  }
  if (overlaps(BOOT_ADDRESS_OFFSET, sizeof(boot_address_value))) {
    if (core == RiscV) {
      std::memcpy(&boot_address_value, &control_registers[core][BOOT_ADDRESS_OFFSET], sizeof(boot_address_value));
      boot_address.write(boot_address_value);
    } else {
      std::memcpy(&vtor_value, &control_registers[core][BOOT_ADDRESS_OFFSET], sizeof(vtor_value));
      vector_table_offset.write(vtor_value);
    }
  }
  if (overlaps(CLOCK_PERIOD_OFFSET, sizeof(clock_period_ns[core]))) {
    std::memcpy(&clock_period_ns[core], &control_registers[core][CLOCK_PERIOD_OFFSET], sizeof(clock_period_ns[core]));
    clock_period[core].write(sc_core::sc_time(clock_period_ns[core], sc_core::SC_NS));
  }
  if (core == Arm && overlaps(CPU_WAIT_OFFSET, 1)) {
    cpu_wait.write(control_registers[core][CPU_WAIT_OFFSET] != 0);
  }
  if (overlaps(SYSTEM_BUS_TRIGGER_OFFSET, sizeof(uint32_t))) {
    uint32_t trigger = 0;
    std::memcpy(&trigger, &control_registers[core][SYSTEM_BUS_TRIGGER_OFFSET], sizeof(trigger));
    if (trigger != 0) {
      system_bus_write_pending[core] = true;
      system_bus_write_event.notify(sc_core::SC_ZERO_TIME);
    }
  }
}

void hmp_signals::system_bus_writer() {
  while (true) {
    wait(system_bus_write_event);
    // Let the triggering Renode transaction finish before issuing requests
    // in the opposite direction. Each bridge retains its own initiator ID.
    wait(sc_core::SC_ZERO_TIME);

    for (int core = 0; core < NUM_CORES; ++core) {
      if (!system_bus_write_pending[core]) {
        continue;
      }
      system_bus_write_pending[core] = false;
      uint32_t value = SYSTEM_BUS_TEST_VALUE + core;
      tlm::tlm_generic_payload payload;
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

      payload.set_command(tlm::TLM_WRITE_COMMAND);
      payload.set_address(SYSTEM_BUS_TEST_ADDRESS[core]);
      payload.set_data_ptr(reinterpret_cast<unsigned char *>(&value));
      payload.set_data_length(sizeof(value));
      payload.set_streaming_width(sizeof(value));
      payload.set_byte_enable_ptr(nullptr);
      payload.set_dmi_allowed(false);
      payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

      system_bus_initiator_socket[core]->b_transport(payload, delay);
      if (payload.is_response_error()) {
        std::cerr << "SystemC-initiated write to Renode failed for core " << core << std::endl;
      }
    }
  }
}
