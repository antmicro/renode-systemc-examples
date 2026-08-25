// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <top.h>

top::top(sc_core::sc_module_name name, const char *addr, const char *port)
    : sc_core::sc_module(name),
      m_conn("m_conn", addr, port),
      m_bridge_m55("m_bridge_m55", addr, port, false, "", "", 0, false, &m_conn),
      m_bridge_m0 ("m_bridge_m0",  addr, port, false, "", "", 1, false, &m_conn),
      m_shared_mem("m_shared_mem", 0x20000000, 0x20000) {
  m_bridge_m55.initiator_socket(m_shared_mem.socket);
  m_bridge_m0.initiator_socket(m_shared_mem.socket);
}
