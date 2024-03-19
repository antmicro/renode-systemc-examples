#pragma once

#include "tlm.h"

#include "peripheral.h"
#include "renode_bridge.h"
#include "fifo_adapter.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_bridge renode;
  Peripheral peripheral;
  fifo_adapter fifo;

  sc_core::sc_signal<bool> irq_signal;
};
