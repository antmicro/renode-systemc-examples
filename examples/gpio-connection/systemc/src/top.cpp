#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_gpio_peripheral("m_gpio_peripheral") {
  m_renode_bridge.initiator_socket(m_gpio_peripheral.bus_target_socket);
  m_renode_bridge.gpio_ports_out[2](m_gpio_in_signal);
  m_gpio_peripheral.gpio_in(m_gpio_in_signal);
}
