#pragma once

#include "tlm.h"

#include "bus.h"
#include "port_interrupter.h"
#include "renode_bridge.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_bridge m_renode_bridge;
  bus m_bus;
  port_interrupter m_port_interrupter0;
  port_interrupter m_port_interrupter1;

  sc_core::sc_signal<bool, sc_core::SC_MANY_WRITERS> m_port_irq_signal0;
  sc_core::sc_signal<bool, sc_core::SC_MANY_WRITERS> m_port_irq_signal1;
};
