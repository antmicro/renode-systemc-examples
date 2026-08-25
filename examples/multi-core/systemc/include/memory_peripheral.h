// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <systemc>
#include <tlm.h>
#include <tlm_utils/multi_passthrough_target_socket.h>

SC_MODULE(Memory) {
    tlm_utils::multi_passthrough_target_socket<Memory> socket;

    Memory(sc_core::sc_module_name name, uint64_t start_addr, uint64_t size);

    ~Memory();

private:
    unsigned char* memory;
    uint64_t start_address;
    uint64_t mem_size;

    bool check_bounds(tlm::tlm_generic_payload& trans, uint64_t& mem_offset);
    void b_transport(int id, tlm::tlm_generic_payload& trans, sc_core::sc_time &delay);
    unsigned int transport_dbg(int id, tlm::tlm_generic_payload& trans);
};
