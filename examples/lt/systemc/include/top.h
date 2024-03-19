#pragma once

#include "SimpleBusLT.h"
#include "at_target_1_phase.h"
#include "initiator_top.h"
#include "lt_target.h"
#include "renode_bridge.h"
#include "tlm.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  SimpleBusLT<3, 2> m_bus;
  at_target_1_phase m_at_and_lt_target_1;
  lt_target m_lt_target_2;
  initiator_top m_initiator_1;
  initiator_top m_initiator_2;
  renode_bridge m_renode_bridge;
};
