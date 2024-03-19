/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http:

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/
#include "top.h"

top::top(sc_core::sc_module_name name, const char *address, const char *port)
    : sc_core::sc_module(name), m_bus("m_bus"),
      m_at_and_lt_target_1("m_at_and_lt_target_1", 201, "memory_socket_1",
                           4 * 1024, 4, sc_core::sc_time(20, sc_core::SC_NS),
                           sc_core::sc_time(100, sc_core::SC_NS),
                           sc_core::sc_time(60, sc_core::SC_NS)),
      m_lt_target_2("m_lt_target_2", 202, "memory_socket_2", 4 * 1024, 4,
                    sc_core::sc_time(10, sc_core::SC_NS),
                    sc_core::sc_time(50, sc_core::SC_NS),
                    sc_core::sc_time(30, sc_core::SC_NS)),
      m_initiator_1("m_initiator_1", 101, 0x0000000000000000,
                    0x0000000010000000),
      m_initiator_2("m_initiator_2", 102, 0x0000000000000000,
                    0x0000000010000000),
      m_renode_bridge("m_renode_bridge", address, port) {
  m_initiator_1.top_initiator_socket(m_bus.target_socket[0]);
  m_initiator_2.top_initiator_socket(m_bus.target_socket[1]);

  m_renode_bridge.initiator_socket(m_bus.target_socket[2]);

  m_bus.initiator_socket[0](m_at_and_lt_target_1.m_memory_socket);
  m_bus.initiator_socket[1](m_lt_target_2.m_memory_socket);
}
