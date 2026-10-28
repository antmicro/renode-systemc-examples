// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>

#include <tlm.h>

#include <renode_bridge.h>

#include "bus.h"
#include "riscv_signals.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  static constexpr unsigned int NUM_INTERRUPTS = riscv_signals::NUM_INTERRUPTS;

  // Keep in sync with SystemCRiscVSignalsUnit.Signal.
  enum Signal {
    NonMaskableInterrupt = 4096,
    Reset = 4097,
  };

  renode_bridge m_renode_bridge;
  bus m_bus;
  riscv_signals m_riscv_signals;

  std::array<sc_core::sc_signal<bool>, NUM_INTERRUPTS> m_interrupt_signals;
  sc_core::sc_signal<bool> m_reset_signal;
  sc_core::sc_signal<bool> m_nmi_signal;
  sc_core::sc_signal<uint64_t> m_boot_address_signal;
  sc_core::sc_signal<sc_core::sc_time> m_clock_period_signal;
};
