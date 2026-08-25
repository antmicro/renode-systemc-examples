// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "tlm.h"

#include "memory_peripheral.h"
#include "renode_bridge.h"

class top : public sc_core::sc_module {
public:
  top(sc_core::sc_module_name name, const char *address, const char *port);

private:
  renode_connection m_conn;
  renode_bridge     m_bridge_m55;
  renode_bridge     m_bridge_m0;
  Memory            m_shared_mem;
};
