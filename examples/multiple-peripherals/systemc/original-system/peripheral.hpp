/** \file 
 *  \brief Contains an example peripheral that is based soley on SystemC.
 * 
 * \ingroup systemc_example
 */
#pragma once

#include <cstdint>
#include <memory>
#include <systemc>
#include <tlm.h>


/** A single peripheral model that has all the functions needed for the example;
 *   multiple instances are used, rather than creating
 *   separate types of peripherals.
 * 
 *  This module contains only standard TLM and port/export interfaces on its boundary, with no need for
 *   any backdoor access or knowledge of module internals.
 *  Any virtual platform should be able to include this peripheral model with little to no
 *   wrapper logic needed, once the basic generic adapters for TLM and SystemC channels is created.
 * 
 * \ingroup systemc_example
*/
class Peripheral : public sc_core::sc_module {
public:
    // Interfaces to outside modules
    tlm::tlm_initiator_socket<> dma{"dma"};
    tlm::tlm_target_socket<>    register_if{"regif"};
    tlm::tlm_target_socket<>    memory_if{"memory"};
    sc_core::sc_export<sc_core::sc_fifo_in_if<std::uint8_t>>          fifo_out{"fifo_out"};
    sc_core::sc_fifo_in<std::uint8_t>                        fifo_in{"fifo_in"};
    sc_core::sc_export<sc_core::sc_signal_in_if<bool>>           transfer_done_irq{"transfer_done_irq"};

    Peripheral(sc_core::sc_module_name name = "peripheral");
    Peripheral(const Peripheral &) = delete; // Disable copying, since sc_module should not be copied


    /** Using PImpl Idiom to hide all internal class details, since only the public interface
     *   is needed for users of this model.
    */
private:
    class Impl;
    std::shared_ptr<Impl> pimpl;
};