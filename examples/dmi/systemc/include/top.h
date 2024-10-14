#pragma once

#include "tlm.h"

#include "dmi_test.h"
#include "renode_bridge.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_bridge m_renode_bridge;
  dmi_test m_dmi_test;

  sc_core::sc_signal<bool> dmi_test_begin;
};
