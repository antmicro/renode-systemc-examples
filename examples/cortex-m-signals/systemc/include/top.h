// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tlm.h>

#include <renode_bridge.h>
#include <signals_peripheral.h>

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_bridge m_renode_bridge;
  signals_peripheral m_signals_peripheral;

  std::array<sc_core::sc_signal<bool>, 480> m_out_nvic_irqs; // int_rq[480]

  sc_core::sc_signal<bool> m_out_non_maskable_interrupt; // nmi_exp
  sc_core::sc_signal<bool> m_out_core_reset_in;          // core_reset_in
  sc_core::sc_signal<bool> m_out_cpu_wait;               // cpu_wait
  sc_core::sc_signal<bool> m_out_power_on_reset;         // m55_poreset_n

  sc_core::sc_signal<bool> m_in_system_reset_request; // O_sysreset_req
  sc_core::sc_signal<bool> m_in_sleeping;             // O_sleeping
  sc_core::sc_signal<bool> m_in_sleep_deep;           // O_sleep_deep
};
