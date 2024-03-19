#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      m_renode_bridge("m_renode_bridge", renode_address, renode_port),
      m_timekeeper("m_timekeeper") {
  m_renode_bridge.initiator_socket(m_timekeeper.target_socket);
}
