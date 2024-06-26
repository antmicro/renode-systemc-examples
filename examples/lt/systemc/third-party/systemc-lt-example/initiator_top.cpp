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
//==============================================================================
/// @file initiator_top.cpp
//
/// @brief Instantiates initiator and traffic_generator
//
/// @details
///    This module performs:
///      1. Instantiation of the traffic_generator and the lt_initiator
///         and the interconnecting sc_fifo's
///      2. Binding of the Interconnect for the components
//
//==============================================================================
//
//  Original Authors:
//    Jack Donovan, ESLX
//    Charles Wilson, ESLX
//    Anna Keist, ESLX
//
//==============================================================================


#include "initiator_top.h"                         // this header file
#include "reporting.h"                             // reporting macro helpers

static const char *filename = "initiator_top.cpp"; ///< filename for reporting

initiator_top::initiator_top
( sc_core::sc_module_name name
, const unsigned int    ID
, sc_dt::uint64         base_address_1
, sc_dt::uint64         base_address_2
) 
  :sc_module           (name) 	         // module instance name
  ,top_initiator_socket                  // Init the socket
      ("top_initiator_socket")
  ,m_ID                (ID)              // initiator ID
  ,m_initiator                           // Init initiator
    ("m_initiator"
    ,ID                                  // ID for reporting
    )
  ,m_traffic_gen                         // Init traffic Generator
    ("m_traffic_gen"
    ,ID                                  // ID for reporting
    ,base_address_1                      // first base address
    ,base_address_2                      // second base address
    ,4
    )

{
  /// Bind ports to m_request_fifo between m_initiator and m_traffic_gen
  m_traffic_gen.request_out_port  (m_request_fifo);
  m_initiator.request_in_port     (m_request_fifo);

  /// Bind ports to m_response_fifo between m_initiator and m_traffic_gen
  m_initiator.response_out_port   (m_response_fifo);
  m_traffic_gen.response_in_port  (m_response_fifo);

  /// Bind initiator-socket to initiator-socket hierarchical connection 
  m_initiator.initiator_socket(top_initiator_socket);
}


void
initiator_top::invalidate_direct_mem_ptr
( sc_dt::uint64      /*start_range*/
, sc_dt::uint64      /*end_range*/
)
{
  std::ostringstream       msg;          // log message
  msg.str ("");
        
  msg << "Initiator: " << m_ID << " Not implemented";
  REPORT_ERROR(filename, __FUNCTION__, msg.str());      
} // end invalidate_direct_mem_ptr


tlm::tlm_sync_enum
initiator_top::nb_transport_bw
( tlm::tlm_generic_payload & /*payload*/
, tlm::tlm_phase &           /*phase*/
, sc_core::sc_time &         /*delta*/
)
{
  std::ostringstream       msg;                 // log message
  msg.str ("");

  msg << "Initiator: " << m_ID 
      << " Not implemented, for hierachical connection of initiator socket";
  REPORT_ERROR(filename, __FUNCTION__, msg.str());

  return tlm::TLM_COMPLETED;  
} // end nb_transport_bw


