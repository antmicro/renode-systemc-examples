// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "top.h"

namespace {
// Match SystemCCortexMSignalsUnit.Signal and SystemCRiscVSignalsUnit.Signal.
namespace cortex_m {
enum Signal : uint32_t {
  NonMaskableInterrupt = 1000,
  CoreResetIn = 1001,
  CpuWait = 1002,
  SystemResetRequest = 1006,
};
}

namespace riscv {
enum Signal : uint32_t {
  NonMaskableInterrupt = 4096,
  Reset = 4097,
};
}
}

top::top(sc_core::sc_module_name name, const char *address, const char *port)
    : sc_core::sc_module(name), m_connection("connection", address, port),
      m_riscv_bridge("riscv_bridge", address, port, false, "", "", 0, false, &m_connection),
      m_arm_bridge("arm_bridge", address, port, false, "", "", 1, false, &m_connection),
      m_riscv_bus("riscv_bus"), m_arm_bus("arm_bus"), m_signals("signals") {
  renode_bridge *bridges[] = {&m_riscv_bridge, &m_arm_bridge};
  bus *buses[] = {&m_riscv_bus, &m_arm_bus};
  const uint32_t reset_pins[] = {riscv::Signal::Reset, cortex_m::Signal::CoreResetIn};
  const uint32_t nmi_pins[] = {riscv::Signal::NonMaskableInterrupt, cortex_m::Signal::NonMaskableInterrupt};

  for (int core = 0; core < hmp_signals::NUM_CORES; ++core) {
    auto &bridge = *bridges[core];
    auto &bus = *buses[core];
    m_signals.system_bus_initiator_socket[core](bridge.target_socket);
    bridge.initiator_socket(bus.renode_target_socket);
    bus.control_initiator_socket(m_signals.control_target_socket[core]);
    bus.memory_initiator_socket(m_signals.memory_target_socket[core]);

    m_signals.reset_in[core](m_reset_signal[core]);
    bridge.gpio_ports_in[reset_pins[core]](m_reset_signal[core]);
    m_signals.nmi_in[core](m_nmi_signal[core]);
    bridge.gpio_ports_in[nmi_pins[core]](m_nmi_signal[core]);

    for (uint32_t irq = 0; irq < hmp_signals::NUM_INTERRUPTS; ++irq) {
      m_signals.interrupt_inputs[core][irq](m_interrupt_signals[core][irq]);
      // Interrupt 0 is reserved for the PLIC but not for the NVIC
      if (core != hmp_signals::RiscV || irq != 0) {
        bridge.gpio_ports_in[irq](m_interrupt_signals[core][irq]);
      }
    }

    m_signals.clock_period[core](m_clock_period_signal[core]);
    bridge.clock_period_in(m_clock_period_signal[core]);
  }

  m_signals.boot_address(m_boot_address_signal);
  m_riscv_bridge.boot_address_in(m_boot_address_signal);
  m_signals.vector_table_offset(m_vtor_signal);
  m_arm_bridge.init_vtor_ns_in(m_vtor_signal);
  m_signals.cpu_wait(m_cpu_wait_signal);
  m_arm_bridge.gpio_ports_in[cortex_m::Signal::CpuWait](m_cpu_wait_signal);
  m_arm_bridge.gpio_ports_out[cortex_m::Signal::SystemResetRequest](m_system_reset_request_signal);
  m_signals.system_reset_request(m_system_reset_request_signal);
}
