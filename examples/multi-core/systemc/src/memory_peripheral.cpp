// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "memory_peripheral.h"
#include <cstring>

Memory::Memory(sc_core::sc_module_name name, uint64_t start_addr, uint64_t size)
    : sc_core::sc_module(name),
      start_address(start_addr),
      mem_size(size)
{
    memory = new unsigned char[size]();  // Zero-initialized
    socket.register_b_transport(this, &Memory::b_transport);
    socket.register_transport_dbg(this, &Memory::transport_dbg);
}

Memory::~Memory() {
    delete[] memory;
}

bool Memory::check_bounds(tlm::tlm_generic_payload& trans, uint64_t& mem_offset) {
    uint64_t addr = trans.get_address();
    unsigned length = trans.get_data_length();
    if (addr < start_address || addr + length > start_address + mem_size) {
        return false;
    }
    mem_offset = addr - start_address;
    return true;
}

void Memory::b_transport(int, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    unsigned char* data = trans.get_data_ptr();
    unsigned length = trans.get_data_length();

    uint64_t mem_offset;
    if (!check_bounds(trans, mem_offset)) {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    if (trans.is_read()) {
        memcpy(data, &memory[mem_offset], length);
    }
    else if (trans.is_write()) {
        memcpy(&memory[mem_offset], data, length);
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    delay += sc_core::sc_time(1, sc_core::SC_US);
}

unsigned int Memory::transport_dbg(int, tlm::tlm_generic_payload& trans) {
    unsigned char* data = trans.get_data_ptr();
    unsigned length = trans.get_data_length();

    uint64_t mem_offset;
    if (!check_bounds(trans, mem_offset)) {
        return 0;
    }

    if (trans.is_read()) {
        memcpy(data, &memory[mem_offset], length);
    }
    else if (trans.is_write()) {
        memcpy(&memory[mem_offset], data, length);
    }
    return length;
}