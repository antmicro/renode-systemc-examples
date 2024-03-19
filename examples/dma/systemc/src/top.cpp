#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_dmac("m_dmac") {
  m_renode_bridge.initiator_socket(m_dmac.bus_target_socket);
  m_renode_bridge.target_socket(m_dmac.bus_initiator_socket);

  m_renode_bridge.gpio_ports_in[1](dma_bus_request);
  m_dmac.bus_request(dma_bus_request);

  m_renode_bridge.gpio_ports_out[2](dma_bus_free);
  m_dmac.bus_free(dma_bus_free);
}
