#include <memory>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

struct peripheral : public sc_core::sc_module {
    peripheral(sc_core::sc_module_name name);

    tlm_utils::simple_initiator_socket<peripheral> bus_initiator_socket;
    tlm_utils::simple_target_socket<peripheral> bus_target_socket;

    tlm_utils::simple_initiator_socket<peripheral> direct_connection_initiator_socket;
    tlm_utils::simple_target_socket<peripheral> direct_connection_target_socket;

private:
    void receive_bus_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);
    void receive_direct_connection_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

    uint64_t value;

    std::unique_ptr<tlm::tlm_generic_payload> payload;
};
