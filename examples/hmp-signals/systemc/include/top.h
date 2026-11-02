// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <renode_bridge.h>

#include "bus.h"
#include "hmp_signals.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_connection m_connection;
  renode_bridge m_riscv_bridge;
  renode_bridge m_arm_bridge;
  bus m_riscv_bus;
  bus m_arm_bus;
  hmp_signals m_signals;

  std::array<sc_core::sc_signal<bool>, hmp_signals::NUM_INTERRUPTS> m_interrupt_signals[hmp_signals::NUM_CORES];
  sc_core::sc_signal<bool> m_reset_signal[hmp_signals::NUM_CORES];
  sc_core::sc_signal<bool> m_nmi_signal[hmp_signals::NUM_CORES];
  sc_core::sc_signal<sc_core::sc_time> m_clock_period_signal[hmp_signals::NUM_CORES];
  sc_core::sc_signal<uint64_t> m_boot_address_signal;
  sc_core::sc_signal<uint32_t> m_vtor_signal;
  sc_core::sc_signal<bool> m_cpu_wait_signal;
  sc_core::sc_signal<bool> m_system_reset_request_signal;
};
