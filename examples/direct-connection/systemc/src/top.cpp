#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      renode_bridge0("renode_bridge", renode_address, renode_port),
      peripheral0("peripheral0")
{
  renode_bridge0.initiator_socket(peripheral0.bus_target_socket);
  renode_bridge0.target_socket(peripheral0.bus_initiator_socket);

  renode_bridge0.direct_connection_initiators[0](peripheral0.direct_connection_target_socket);
  renode_bridge0.direct_connection_targets[0](peripheral0.direct_connection_initiator_socket);
}

