/**
 * Copyright (c) 2024 Antmicro
 * Copyright (c) 2024 Silicon Labs
*/

/** \file 
 *  \brief This header just defines certain platform API functions accessible by software.
 * 
 * The definitions of these functions must be linked in as part of the platform definition.
 * 
 * \ingroup systemc_example
*/
#pragma once

#include <cstdint>
#include <cstddef> // provides offsetof()


// Virtual platform support functions for accessing memory or peripherals over the system bus
// Embedded platforms would just inline the memcpy directly, but here
//  since using simple CPU model to execute the code, have to map the 
//  accesses to TLM directly (Renode would need something like this to handle bus accesses)
std::uintptr_t systembus_memcpy_write(std::uintptr_t dest, const void * src, std::size_t count);
void * systembus_memcpy_read(void * dest, std::uintptr_t src, std::size_t count);

/** Invoke wait-for-interrupt routine on CPU which is currently executing */
void cpu_wfi();
std::uintptr_t get_peripheral_address(int index);





// Helper functions and macros for reading and writing registers
// These map all accesses to systembus platform API
//  (which would be mostly passthru inline functions for embedded or ISS platforms)
#include "peripheral_register_interface.hpp"

template<typename FieldType, typename ValueType>
void write_peripheral_field(int peripheral_index, int offset, const ValueType & source_value) {
    FieldType field_value = source_value;  // First assign to local value having the correct type, in case types don't match

    static_assert(std::is_copy_assignable_v<FieldType>, "Can't assign field using memcpy");
    systembus_memcpy_write(get_peripheral_address(peripheral_index) + offset, &field_value, sizeof(field_value));
}

template<typename FieldType>
auto read_peripheral_field(int peripheral_index, int offset) {
    FieldType field_value;

    static_assert(std::is_copy_assignable_v<FieldType>, "Can't assign field using memcpy");
    systembus_memcpy_read(&field_value, get_peripheral_address(peripheral_index) + offset, sizeof(field_value));

    return field_value;
}


#define WRITE_PERIPHERAL_FIELD(peripheral_index, field_name, value) \
    write_peripheral_field<decltype(PeripheralRegisterInterface::field_name)>(peripheral_index, offsetof(PeripheralRegisterInterface, field_name), value);

#define READ_PERIPHERAL_FIELD(peripheral_index, field_name) \
    read_peripheral_field<decltype(PeripheralRegisterInterface::field_name)>(peripheral_index, offsetof(PeripheralRegisterInterface, field_name))
