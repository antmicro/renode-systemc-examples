// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address, const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_bus("m_bus"), m_riscv_signals("m_riscv_signals") {
  // SystemC -> Renode system bus.
  m_riscv_signals.system_bus_initiator_socket(m_renode_bridge.target_socket);

  // Renode -> SystemC. The bus dispatches accesses between the control
  // space and the memory.
  m_renode_bridge.initiator_socket(m_bus.renode_target_socket);
  m_bus.control_initiator_socket(m_riscv_signals.control_target_socket);
  m_bus.memory_initiator_socket(m_riscv_signals.memory_target_socket);

  // Signals driven by SystemC towards Renode.
  m_riscv_signals.reset_in(m_reset_signal);
  m_renode_bridge.gpio_ports_in[Signal::Reset](m_reset_signal);

  m_riscv_signals.nmi_in(m_nmi_signal);
  m_renode_bridge.gpio_ports_in[Signal::NonMaskableInterrupt](m_nmi_signal);

  for (unsigned i = 0; i < NUM_INTERRUPTS; ++i) {
    m_riscv_signals.interrupt_inputs[i](m_interrupt_signals[i]);
    m_renode_bridge.gpio_ports_in[i](m_interrupt_signals[i]);
  }

  m_riscv_signals.boot_address(m_boot_address_signal);
  m_renode_bridge.boot_address_in(m_boot_address_signal);

  m_riscv_signals.clock_period(m_clock_period_signal);
  m_renode_bridge.clock_period_in(m_clock_period_signal);
}
