/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

#include "tlm.h"
#include "top.h"
#define REPORT_DEFINE_GLOBALS
#include "reporting.h"
int sc_main(int, char **argv) {
  REPORT_ENABLE_ALL_REPORTING();
  const char *renode_address = argv[1];
  const char *renode_port = argv[2];
  top top("top", renode_address, renode_port);

  sc_core::sc_start();

  return 0;
}
