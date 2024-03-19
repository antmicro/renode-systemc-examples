#pragma once

#include "tlm.h"

#include "dmac.h"
#include "renode_bridge.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_bridge m_renode_bridge;
  dmac m_dmac;

  sc_core::sc_signal<bool> dma_interrupt;
  sc_core::sc_signal<bool> dma_bus_request;
  sc_core::sc_signal<bool> dma_bus_free;
};
