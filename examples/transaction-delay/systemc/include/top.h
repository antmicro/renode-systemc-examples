#pragma once

#include "tlm.h"

#include "renode_bridge.h"
#include "timekeeper.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *renode_address,
      const char *renode_port);

private:
  renode_bridge m_renode_bridge;
  timekeeper m_timekeeper;
};
