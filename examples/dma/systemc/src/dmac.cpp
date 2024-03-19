#include "dmac.h"

#include "tlm.h"

#include <stdio.h>

SC_HAS_PROCESS(dmac);
dmac::dmac(sc_core::sc_module_name name) {
  SC_THREAD(dmac_loop);
  sensitive << start_event;

  bus_target_socket.register_b_transport(this, &dmac::receive_b_transport);

  payload.reset(new tlm::tlm_generic_payload());
}

static void reset_payload(tlm::tlm_generic_payload &payload) {
  // Reset the TLM payload to reuse it between requests.
  payload.set_command(tlm::TLM_IGNORE_COMMAND);
  payload.set_address(0);
  payload.set_data_ptr(0);
  payload.set_data_length(1);
  payload.set_streaming_width(1);
  payload.set_byte_enable_ptr(nullptr);
  payload.set_byte_enable_length(0);
  payload.set_dmi_allowed(false);
  payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
}

void dmac::dmac_loop() {
  uint8_t data;
  while (true) {
    // Wait for "start_event" which is raised when an appropriate value is
    // written to the control register.
    sc_core::wait();

    // Request access to the bus.
    bus_request.write(true);
    printf("DMAC: bus request sent, waiting for bus free.\n");
    sc_core::wait(bus_free.posedge_event());
    printf("DMAC: bus free; start memory copy\n");

    // Copy the memory contents byte by byte.
    uint32_t r_addr = addr_src;
    uint32_t w_addr = addr_dst;
    for (int i = 0; i < data_len; ++i) {
      // Read byte from source address.
      reset_payload(*payload);
      payload->set_command(tlm::TLM_READ_COMMAND);
      payload->set_address(r_addr);
      payload->set_data_ptr(&data);

      //    Make a read request to the system bus.
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
      bus_initiator_socket->b_transport(*payload, delay);
      if (payload->get_response_status() != tlm::TLM_OK_RESPONSE) {
        fprintf(stderr, "Failed to read from system bus\n");
        assert(false);
        return;
      }
      //    Payload data now contains the value to be copied.

      // Write byte to destination address.
      reset_payload(*payload);
      payload->set_command(tlm::TLM_WRITE_COMMAND);
      payload->set_address(w_addr);
      payload->set_data_ptr(&data);

      //    Make a write request to the system bus.
      bus_initiator_socket->b_transport(*payload, delay);
      if (payload->get_response_status() != tlm::TLM_OK_RESPONSE) {
        fprintf(stderr, "Failed to write to system bus\n");
        assert(false);
        return;
      }

      ++r_addr;
      ++w_addr;
    }

    // Notify the system that the bus is now free.
    bus_request.write(false);

    printf("DMAC: copy complete\n");
  }
}

void dmac::receive_b_transport(tlm::tlm_generic_payload &trans,
                               sc_core::sc_time &delay) {
  // Service a request from the system bus.

  // For simplicity all the registers are write-only.
  assert(trans.is_write());

  uint32_t address = trans.get_address();
  uint32_t value = *((uint32_t *)trans.get_data_ptr());
  switch (address) {
  case 0x4: {
    // Source address register.
    printf("DMAC: setting source address to 0x%08X\n", value);
    addr_src = value;
  } break;
  case 0x8: {
    // Destination address register.
    printf("DMAC: setting destination address to 0x%08X\n", value);
    addr_dst = value;
  } break;
  case 0xC: {
    // Data length register.
    printf("DMAC: setting data length to %u bytes\n", value);
    data_len = value;
  } break;
  case 0x10: {
    // Control register.
    switch (value & 0xf) {
    case 1: {
      // Start transfer.
      start_event.notify();
    } break;
    default:
      fprintf(stderr, "Unhandled control register value: 0x%08X", value);
      assert(false);
    }
  } break;
  default:
    assert(!"Unhandled address");
  }
}
