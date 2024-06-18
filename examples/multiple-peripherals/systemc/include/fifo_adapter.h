#pragma once

#include <memory>
#include <cstdint>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

struct fifo_adapter: public sc_core::sc_module {
    fifo_adapter(sc_core::sc_module_name name);

    tlm_utils::simple_initiator_socket<fifo_adapter> fifo_initiator;
    tlm_utils::simple_target_socket<fifo_adapter> fifo_target;

    sc_core::sc_fifo_in<std::uint8_t> fifo_in{"fifo_in"};

    sc_core::sc_export<sc_core::sc_fifo_in_if<std::uint8_t>> fifo_out{"fifo_out"};

private:
    void forward_fifo();
    void on_fifo_in(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);

    sc_core::sc_fifo<uint8_t> fifo{"fifo"};
    std::unique_ptr<tlm::tlm_generic_payload> payload;
};
