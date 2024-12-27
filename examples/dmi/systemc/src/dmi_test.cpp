#include "dmi_test.h"

#include "tlm.h"

#include <cstring>
#include <stdio.h>

SC_HAS_PROCESS(dmi_test);
dmi_test::dmi_test(sc_core::sc_module_name name, renode_bridge &renode_bridge)
    : m_renode_bridge(renode_bridge) {
  SC_THREAD(dmi_test_sequence);

  payload.reset(new tlm::tlm_generic_payload());
}

static void reset_payload(tlm::tlm_generic_payload &payload) {
  // Reset the TLM payload to reuse it between requests.
  payload.set_command(tlm::TLM_IGNORE_COMMAND);
  payload.set_address(0);
  payload.set_data_ptr(nullptr);
  payload.set_data_length(1);
  payload.set_streaming_width(1);
  payload.set_byte_enable_ptr(nullptr);
  payload.set_byte_enable_length(0);
  payload.set_dmi_allowed(false);
  payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
}

void dmi_test::dmi_test_sequence() {
  uint32_t write_data_word;
  uint32_t read_data_word = 0;
  uint64_t test_address;
  tlm::tlm_dmi dmi_data;
  while (true) {
    sc_core::wait(test_begin.posedge_event());

    puts("First test: write via b_transport, read via DMI\n");
    test_address = 0x20001230;
    write_data_word = 0xb1e55ed;

    // Write test value using b_transport
    reset_payload(*payload);
    payload->set_command(tlm::TLM_WRITE_COMMAND);
    payload->set_address(test_address);
    payload->set_data_ptr(reinterpret_cast<unsigned char *>(&write_data_word));
    payload->set_data_length(4);
    payload->set_streaming_width(4);

    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    bus_initiator_socket->b_transport(*payload, delay);
    if (payload->get_response_status() != tlm::TLM_OK_RESPONSE) {
      fprintf(stderr, "Failed to write to system bus\n");
      assert(false);
      return;
    }
    if (!payload->is_dmi_allowed()) {
      fprintf(stderr, "Target did not set DMI hint\n");
      assert(false);
      return;
    }

    // Acquire DMI pointer for test address
    bool dmi_ptr_valid =
        bus_initiator_socket->get_direct_mem_ptr(*payload, dmi_data);
    if (!dmi_ptr_valid) {
      fprintf(stderr, "get_direct_mem_ptr() did not succeed\n");
      assert(false);
      return;
    }

    // read back the value using DMI
    std::memcpy(&read_data_word,
                dmi_data.get_dmi_ptr() +
                    (test_address - dmi_data.get_start_address()),
                4);
    if (read_data_word == write_data_word) {
      puts("DMI read data matched\n");
    } else {
      fprintf(stderr, "DMI read data mismatched\n");
      assert(false);
      return;
    }

    puts("Second test: write via b_transport, read via DMI\n");
    test_address = 0x20002468;
    write_data_word = 0xc0ffee;

    // Acquire DMI pointer for test address
    payload->set_address(test_address);
    dmi_ptr_valid =
        bus_initiator_socket->get_direct_mem_ptr(*payload, dmi_data);
    if (!dmi_ptr_valid) {
      fprintf(stderr, "get_direct_mem_ptr() did not succeed\n");
      assert(false);
      return;
    }

    // write test value using DMI
    std::memcpy(dmi_data.get_dmi_ptr() +
                    (test_address - dmi_data.get_start_address()),
                &write_data_word, 4);

    // For example's sake we're assuming the memory we've written to through
    // DMI contains code. In this case we need to invalidate translation
    // blocks in Renode to avoid unexpected behavior.
    m_renode_bridge.invalidate_translation_blocks(test_address,
                                                  test_address + 4);

    // read back value using b_transport
    reset_payload(*payload);
    payload->set_command(tlm::TLM_READ_COMMAND);
    payload->set_address(test_address);
    payload->set_data_ptr(reinterpret_cast<unsigned char *>(&read_data_word));
    payload->set_data_length(4);
    payload->set_streaming_width(4);

    bus_initiator_socket->b_transport(*payload, delay);
    if (payload->get_response_status() != tlm::TLM_OK_RESPONSE) {
      fprintf(stderr, "Failed to read from system bus\n");
      assert(false);
      return;
    }

    if (read_data_word == write_data_word) {
      puts("DMI write data matched\n");
    } else {
      fprintf(stderr, "DMI write data mismatched\n");
      assert(false);
      return;
    }
  }
}
