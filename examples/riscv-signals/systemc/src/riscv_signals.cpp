// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "riscv_signals.h"

#include <cstring>
#include <iostream>

SC_HAS_PROCESS(riscv_signals);
riscv_signals::riscv_signals(sc_core::sc_module_name name)
    : sc_module(name), boot_address_value(DEFAULT_BOOT_ADDRESS),
      clock_period_ns(DEFAULT_CLOCK_PERIOD_NS) {
  reset_in.initialize(false);
  nmi_in.initialize(false);
  boot_address.initialize(boot_address_value);
  clock_period.initialize(sc_core::sc_time(clock_period_ns, sc_core::SC_NS));

  for (auto &interrupt_input : interrupt_inputs) {
    interrupt_input.initialize(false);
  }

  std::memcpy(&control_registers[BOOT_ADDRESS_OFFSET], &boot_address_value, sizeof(boot_address_value));
  std::memcpy(&control_registers[CLOCK_PERIOD_OFFSET], &clock_period_ns, sizeof(clock_period_ns));

  control_target_socket.register_b_transport(this, &riscv_signals::handle_control);
  memory_target_socket.register_b_transport(this, &riscv_signals::handle_memory);

  SC_THREAD(system_bus_writer);
}

void riscv_signals::handle_control(tlm::tlm_generic_payload &payload, sc_core::sc_time &) {
  const auto offset = payload.get_address();
  const auto length = payload.get_data_length();

  if (offset + length > control_registers.size()) {
    payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  if (payload.is_read()) {
    std::memcpy(payload.get_data_ptr(), &control_registers[offset], length);
  } else if (payload.is_write()) {
    std::memcpy(&control_registers[offset], payload.get_data_ptr(), length);
    apply_control_write(offset, length);
  } else {
    payload.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return;
  }

  payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

void riscv_signals::handle_memory(tlm::tlm_generic_payload &payload, sc_core::sc_time &) {
  const auto offset = payload.get_address();
  const auto length = payload.get_data_length();

  if (offset + length > memory.size()) {
    payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  if (payload.is_read()) {
    std::memcpy(payload.get_data_ptr(), &memory[offset], length);
  } else if (payload.is_write()) {
    std::memcpy(&memory[offset], payload.get_data_ptr(), length);
  } else {
    payload.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return;
  }

  payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

void riscv_signals::apply_control_write(uint64_t offset, unsigned length) {
  const auto overlaps = [offset, length](uint64_t start, uint64_t size) {
    return offset < start + size && start < offset + length;
  };

  for (unsigned i = 0; i < NUM_INTERRUPTS; ++i) {
    if (overlaps(i, 1)) {
      interrupt_inputs[i].write(control_registers[i] != 0);
    }
  }

  if (overlaps(RESET_OFFSET, 1)) {
    reset_in.write(control_registers[RESET_OFFSET] != 0);
  }
  if (overlaps(NMI_OFFSET, 1)) {
    nmi_in.write(control_registers[NMI_OFFSET] != 0);
  }
  if (overlaps(BOOT_ADDRESS_OFFSET, sizeof(boot_address_value))) {
    std::memcpy(&boot_address_value, &control_registers[BOOT_ADDRESS_OFFSET], sizeof(boot_address_value));
    boot_address.write(boot_address_value);
  }
  if (overlaps(CLOCK_PERIOD_OFFSET, sizeof(clock_period_ns))) {
    std::memcpy(&clock_period_ns, &control_registers[CLOCK_PERIOD_OFFSET], sizeof(clock_period_ns));
    clock_period.write(sc_core::sc_time(clock_period_ns, sc_core::SC_NS));
  }
  if (overlaps(SYSTEM_BUS_TRIGGER_OFFSET, sizeof(uint32_t))) {
    uint32_t trigger = 0;
    std::memcpy(&trigger, &control_registers[SYSTEM_BUS_TRIGGER_OFFSET], sizeof(trigger));
    if (trigger != 0) {
      system_bus_write_pending = true;
      system_bus_write_event.notify(sc_core::SC_ZERO_TIME);
    }
  }
}

void riscv_signals::system_bus_writer() {
  while (true) {
    if (!system_bus_write_pending) {
      wait(system_bus_write_event);
    }
    system_bus_write_pending = false;

    // Let the Renode transaction that triggered us finish before issuing
    // a request in the opposite direction.
    wait(sc_core::SC_ZERO_TIME);

    uint32_t value = SYSTEM_BUS_TEST_VALUE;
    tlm::tlm_generic_payload payload;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

    payload.set_command(tlm::TLM_WRITE_COMMAND);
    payload.set_address(SYSTEM_BUS_TEST_ADDRESS);
    payload.set_data_ptr(reinterpret_cast<unsigned char *>(&value));
    payload.set_data_length(sizeof(value));
    payload.set_streaming_width(sizeof(value));
    payload.set_byte_enable_ptr(nullptr);
    payload.set_dmi_allowed(false);
    payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    system_bus_initiator_socket->b_transport(payload, delay);

    if (payload.is_response_error()) {
      std::cerr << "SystemC-initiated write to Renode failed" << std::endl;
    }
  }
}
