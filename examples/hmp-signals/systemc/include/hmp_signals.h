// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <cstdint>

#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

class hmp_signals : public sc_core::sc_module {
public:
  enum Core { RiscV = 0, Arm = 1, NUM_CORES = 2 };
  static constexpr unsigned NUM_INTERRUPTS = 16;

  // Renode system bus address and value used by the SystemC-initiated
  // transaction test. The address points to the Renode RAM in the example
  // platform.
  static constexpr uint64_t SYSTEM_BUS_TEST_ADDRESS[NUM_CORES] = {0x80001000, 0x20001000};
  static constexpr uint32_t SYSTEM_BUS_TEST_VALUE = 0x5A5AA5A5;

  // Control register offsets. Addresses below 0x1000 are routed to the
  // control target socket.
  static constexpr uint64_t RESET_OFFSET = 0x100;
  static constexpr uint64_t NMI_OFFSET = 0x104;
  static constexpr uint64_t BOOT_ADDRESS_OFFSET = 0x108;
  static constexpr uint64_t CLOCK_PERIOD_OFFSET = 0x110;
  static constexpr uint64_t SYSTEM_BUS_TRIGGER_OFFSET = 0x114;

  static constexpr uint64_t CPU_WAIT_OFFSET = 0x118;
  static constexpr uint64_t SYSTEM_RESET_REQUEST_OFFSET = 0x11c;
  static constexpr uint32_t DEFAULT_VTOR = 0x20000000;

  static constexpr uint64_t DEFAULT_BOOT_ADDRESS = 0x80000000;
  static constexpr uint32_t DEFAULT_CLOCK_PERIOD_NS[NUM_CORES] = {10, 20};

  hmp_signals(sc_core::sc_module_name name);

  // Signals driven by SystemC and consumed by the Renode signals unit.
  sc_core::sc_out<bool> reset_in[NUM_CORES];
  sc_core::sc_out<bool> nmi_in[NUM_CORES];
  sc_core::sc_out<uint64_t> boot_address;
  sc_core::sc_out<bool> interrupt_inputs[NUM_CORES][NUM_INTERRUPTS];
  sc_core::sc_out<sc_core::sc_time> clock_period[NUM_CORES];
  sc_core::sc_out<uint32_t> vector_table_offset;
  sc_core::sc_out<bool> cpu_wait;
  sc_core::sc_in<bool> system_reset_request;

  // Initiator used by SystemC to access the Renode system bus.
  tlm_utils::simple_initiator_socket<hmp_signals> system_bus_initiator_socket[NUM_CORES];

  // Target socket for Renode-initiated memory transactions.
  tlm_utils::simple_target_socket_tagged<hmp_signals> memory_target_socket[NUM_CORES];

  // Target socket used to drive the signals from Renode (PoC control plane).
  tlm_utils::simple_target_socket_tagged<hmp_signals> control_target_socket[NUM_CORES];

private:
  static constexpr uint64_t CONTROL_SIZE = 0x1000;
  static constexpr uint64_t MEMORY_SIZE = 0x1000;

  void handle_control(int core, tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);
  void handle_memory(int core, tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);
  void apply_control_write(int core, uint64_t offset, unsigned length);
  void system_bus_writer();

  std::array<uint8_t, CONTROL_SIZE> control_registers[NUM_CORES]{};
  std::array<uint8_t, MEMORY_SIZE> memory[NUM_CORES]{};

  sc_core::sc_event system_bus_write_event;
  bool system_bus_write_pending[NUM_CORES]{};

  uint64_t boot_address_value;
  uint32_t clock_period_ns[NUM_CORES];
  uint32_t vtor_value;
};
