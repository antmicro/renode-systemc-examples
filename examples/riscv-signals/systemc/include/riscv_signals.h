// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <cstdint>

#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

class riscv_signals : public sc_core::sc_module {
public:
  static constexpr unsigned NUM_INTERRUPTS = 16;

  // Renode system bus address and value used by the SystemC-initiated
  // transaction test. The address points to the Renode RAM in the example
  // platform.
  static constexpr uint64_t SYSTEM_BUS_TEST_ADDRESS = 0x80001000;
  static constexpr uint32_t SYSTEM_BUS_TEST_VALUE = 0x5A5AA5A5;

  // Control register offsets. Addresses below 0x1000 are routed to the
  // control target socket.
  static constexpr uint64_t RESET_OFFSET = 0x100;
  static constexpr uint64_t NMI_OFFSET = 0x104;
  static constexpr uint64_t BOOT_ADDRESS_OFFSET = 0x108;
  static constexpr uint64_t CLOCK_PERIOD_OFFSET = 0x110;
  static constexpr uint64_t SYSTEM_BUS_TRIGGER_OFFSET = 0x114;

  static constexpr uint64_t DEFAULT_BOOT_ADDRESS = 0x80000000;
  static constexpr uint32_t DEFAULT_CLOCK_PERIOD_NS = 10;

  riscv_signals(sc_core::sc_module_name name);

  // Signals driven by SystemC and consumed by the Renode signals unit.
  sc_core::sc_out<bool> reset_in;
  sc_core::sc_out<bool> nmi_in;
  sc_core::sc_out<uint64_t> boot_address;
  sc_core::sc_out<bool> interrupt_inputs[NUM_INTERRUPTS];
  sc_core::sc_out<sc_core::sc_time> clock_period;

  // Initiator used by SystemC to access the Renode system bus.
  tlm_utils::simple_initiator_socket<riscv_signals> system_bus_initiator_socket;

  // Target socket for Renode-initiated memory transactions.
  tlm_utils::simple_target_socket<riscv_signals> memory_target_socket;

  // Target socket used to drive the signals from Renode (PoC control plane).
  tlm_utils::simple_target_socket<riscv_signals> control_target_socket;

private:
  static constexpr uint64_t CONTROL_SIZE = 0x1000;
  static constexpr uint64_t MEMORY_SIZE = 0x1000;

  void handle_control(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);
  void handle_memory(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);
  void apply_control_write(uint64_t offset, unsigned length);
  void system_bus_writer();

  std::array<uint8_t, CONTROL_SIZE> control_registers{};
  std::array<uint8_t, MEMORY_SIZE> memory{};

  sc_core::sc_event system_bus_write_event;
  bool system_bus_write_pending = false;

  uint64_t boot_address_value;
  uint32_t clock_period_ns;
};
