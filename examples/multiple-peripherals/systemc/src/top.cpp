#include "top.h"

top::top(sc_core::sc_module_name name, const char *renode_address,
         const char *renode_port)
    : sc_core::sc_module(name),
      renode("renode", renode_address, renode_port),
      peripheral("peripheral"),
      fifo("fifo")
{
  peripheral.register_if(renode.initiator_socket);

  renode.gpio_ports_in[0](peripheral.transfer_done_irq);

  fifo.fifo_in.bind(peripheral.fifo_out);
  peripheral.fifo_in.bind(fifo.fifo_out);

  peripheral.memory_if(renode.direct_connection_initiators[0]);
  peripheral.dma(renode.direct_connection_targets[0]);
  
  renode.direct_connection_initiators[1](fifo.fifo_target);
  renode.direct_connection_targets[1](fifo.fifo_initiator);
}
