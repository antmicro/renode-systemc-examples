#include "peripheral.h"

peripheral::peripheral(sc_core::sc_module_name name) {
    bus_target_socket.register_b_transport(this, &peripheral::receive_bus_b_transport);

    direct_connection_target_socket.register_b_transport(this, &peripheral::receive_direct_connection_b_transport);

    payload.reset(new tlm::tlm_generic_payload());
}

void peripheral::receive_bus_b_transport(tlm::tlm_generic_payload &trans,
                               sc_core::sc_time &delay) {
    if(trans.is_write()) {
      if(trans.get_address() == 0) {
          value = *((uint64_t*)trans.get_data_ptr());
          trans.set_response_status(tlm::TLM_OK_RESPONSE);
      } else {
          uint64_t data = value;
          payload->set_command(tlm::TLM_WRITE_COMMAND);
          payload->set_address(*((uint64_t*)trans.get_data_ptr()));
          payload->set_data_ptr((uint8_t*)&data);
          payload->set_data_length(8);
          payload->set_streaming_width(8);
          payload->set_byte_enable_ptr(nullptr);
          payload->set_byte_enable_length(0);
          payload->set_dmi_allowed(false);
          payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
          sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
          tlm::tlm_phase phase = tlm::BEGIN_REQ;
          auto ret_val = bus_initiator_socket->nb_transport_fw(*payload, phase, delay);
          assert(ret_val == tlm::TLM_COMPLETED);
      }
    } else {
      // If this is a read, communicate with the other peripheral through a direct connection,
      // and return a value from it.
      uint64_t data = 0;
      payload->set_command(tlm::TLM_READ_COMMAND);
      payload->set_address(0);
      payload->set_data_ptr((uint8_t*)&data);
      payload->set_data_length(8);
      payload->set_streaming_width(8);
      payload->set_byte_enable_ptr(nullptr);
      payload->set_byte_enable_length(0);
      payload->set_dmi_allowed(false);
      payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      auto ret_val = direct_connection_initiator_socket->nb_transport_fw(*payload, phase, delay);
      assert(ret_val == tlm::TLM_COMPLETED);

      uint64_t recv_value = *((uint64_t*)payload->get_data_ptr());

      uint64_t* data_ptr = (uint64_t*)trans.get_data_ptr();
      *data_ptr = recv_value;
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
}

void peripheral::receive_direct_connection_b_transport(tlm::tlm_generic_payload &trans,
                               sc_core::sc_time &delay) {
    // Ignore write requests on the direct channel.
    if(trans.is_read()) {
      uint64_t* data_ptr = (uint64_t*)trans.get_data_ptr();
      *data_ptr = value;
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
}
