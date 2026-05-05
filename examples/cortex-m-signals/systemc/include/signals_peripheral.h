// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

class signals_peripheral : public sc_core::sc_module {
public:
  signals_peripheral(sc_core::sc_module_name name);

  tlm_utils::simple_target_socket<signals_peripheral> bus_target_socket;

  std::array<sc_core::sc_out<bool>, 480> out_nvic_irqs{}; // int_rq[480]

  sc_core::sc_out<bool> out_non_maskable_interrupt; // nmi_exp
  sc_core::sc_out<bool> out_core_reset_in;          // core_reset_in
  sc_core::sc_out<bool> out_cpu_wait;               // cpu_wait
  sc_core::sc_out<bool> out_power_on_reset;         // m55_poreset_n
  sc_core::sc_out<uint32_t> out_init_s_vtor;        // m55_initsvtor
  sc_core::sc_out<uint32_t> out_init_ns_vtor;       // init_ns_vtor

  sc_core::sc_in<bool> in_system_reset_request; // O_sysreset_req
  sc_core::sc_in<bool> in_sleeping;             // O_sleeping
  sc_core::sc_in<bool> in_sleep_deep;           // O_sleep_deep

private:
  void receive_b_transport(tlm::tlm_generic_payload &trans,
                           sc_core::sc_time &delay);
  void on_gpio_in();
  void on_system_reset_request();
  void handle_read(tlm::tlm_generic_payload &trans);
  void handle_write(tlm::tlm_generic_payload &trans);

  // Should be kept in sync with the variables in the robot file
  const uint32_t vector_table_offset_non_secure = 0x2000a000;
  const uint32_t vector_table_offset = 0x2000b000;
  // The memory_offset should match the repl config
  const uint32_t memory_offset = 0x20000000;
};

enum Signal {
  NvicIrqsStart = 0,                     // int_rq[0]
  NvicIrqsEnd = 479,                     // int_rq[479]
  NonMaskableInterrupt = 1000,           // nmi_exp
  CoreResetIn = 1001,                    // core_reset_in
  CpuWait = 1002,                        // cpu_wait
  InitNonSecureVectorTableOffset = 1003, // init_ns_vtor
  InitSecureVectorTableOffset = 1004,    // m55_initsvtor
  PowerOnReset = 1005,                   // m55_poreset_n
  SystemResetRequest = 1006,             // O_sysreset_req
  Sleeping = 1007,                       // O_sleeping
  SleepDeep = 1008,                      // O_sleep_deep
};

constexpr uint64_t TRIGGER_SIGNAL_OFFSET = 0x100;
