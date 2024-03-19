#pragma once

#include "tlm.h"

#include "peripheral.h"
#include "renode_bridge.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  peripheral peripheral0;
  renode_bridge renode_bridge0;
};
