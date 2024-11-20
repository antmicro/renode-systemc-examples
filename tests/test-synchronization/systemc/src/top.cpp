#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_writer("m_writer", 0x1000000) {
  m_renode_bridge.target_socket(m_writer.bus_initiator_socket);
}
