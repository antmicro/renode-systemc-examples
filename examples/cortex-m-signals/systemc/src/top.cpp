#include <top.h>

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_signals_peripheral("m_signals_peripheral") {
  m_renode_bridge.initiator_socket(m_signals_peripheral.bus_target_socket);

  m_renode_bridge.gpio_ports_in[Signal::NonMaskableInterrupt](
      m_out_non_maskable_interrupt);
  m_renode_bridge.gpio_ports_in[Signal::CoreResetIn](m_out_core_reset_in);
  m_renode_bridge.gpio_ports_in[Signal::CpuWait](m_out_cpu_wait);
  m_renode_bridge.gpio_ports_in[Signal::PowerOnReset](m_out_power_on_reset);

  m_signals_peripheral.out_non_maskable_interrupt(m_out_non_maskable_interrupt);
  m_signals_peripheral.out_core_reset_in(m_out_core_reset_in);
  m_signals_peripheral.out_cpu_wait(m_out_cpu_wait);
  m_signals_peripheral.out_power_on_reset(m_out_power_on_reset);

  m_renode_bridge.gpio_ports_out[Signal::SystemResetRequest](
      m_in_system_reset_request);
  m_renode_bridge.gpio_ports_out[Signal::Sleeping](m_in_sleeping);
  m_renode_bridge.gpio_ports_out[Signal::SleepDeep](m_in_sleep_deep);

  m_signals_peripheral.in_system_reset_request(m_in_system_reset_request);
  m_signals_peripheral.in_sleeping(m_in_sleeping);
  m_signals_peripheral.in_sleep_deep(m_in_sleep_deep);
}
