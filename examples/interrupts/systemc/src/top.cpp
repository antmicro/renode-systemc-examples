#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_port_interrupter0("m_port_interrupter0", 1),
      m_port_interrupter1("m_port_interrupter1", 3), m_bus("m_bus") {
  // Renode-initiated writes resetting the interrupt signals are routed to
  // through the bus to the relevant interrupter.
  m_renode_bridge.initiator_socket(m_bus.renode_target_socket);
  m_bus.initiator_socket0(m_port_interrupter0.target_socket);
  m_bus.initiator_socket1(m_port_interrupter1.target_socket);

  // Interrupt signals connect directly to renode_bridge.
  m_port_interrupter0.irq_out(m_port_irq_signal0);
  m_renode_bridge.gpio_ports_in[0](m_port_irq_signal0);

  m_port_interrupter1.irq_out(m_port_irq_signal1);
  m_renode_bridge.gpio_ports_in[1](m_port_irq_signal1);
}
