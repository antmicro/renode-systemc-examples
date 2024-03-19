/** \file
 *  \brief Contains the register interface API for the peripheral.
 * 
 *  The model is such that this header can currently be shared by both model and software,
 *   with both having this same source of truth.
 * 
 * \ingroup systemc_example
*/
#pragma once

#include <cstdint>
#include <array>

/** Struct defining the register interface API for the peripheral
 * 
 *  Instead of defining a CMSIS-like serialization of the peripheral
 *   register interface, just use a struct-based encoding so don't
 *   have to serialize and deserialize.
 *  An equivalent CMSIS-like mapping of the fields could be used but
 *   is unnecessary.
 * 
 * \ingroup systemc_example
 * */
struct PeripheralRegisterInterface {
    bool cmd_start_timer = false;
    std::uint64_t timer = 0;
    bool timer_done = false;
    bool irq_ack = false;

    // peripheral memory also aliased into the register interface
    std::array<uint8_t, 100> memory;
};
