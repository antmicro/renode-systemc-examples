#pragma once

#include "tlm.h"

#include "gpio_peripheral.h"
#include "renode_bridge.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_bridge m_renode_bridge;
  gpio_peripheral m_gpio_peripheral;

  sc_core::sc_signal<bool> m_gpio_in_signal;
  sc_core::sc_signal<bool> m_gpio_out_signal;
};
