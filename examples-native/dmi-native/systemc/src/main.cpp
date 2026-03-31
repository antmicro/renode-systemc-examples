#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <cassert>
#include <cstring>

#include "dmi_test.h"
#include "renode_bridge_native.h"

// Module that forwards all traffic to Renode: TLM transfers via bus access methods,
// DMI requests via renode_get_direct_mem_ptr
struct renode_memory_transport : sc_core::sc_module {
  tlm_utils::simple_target_socket<renode_memory_transport> socket;

  renode_memory_transport(sc_core::sc_module_name name) : sc_module(name), socket("socket") {
    socket.register_b_transport(this, &renode_memory_transport::b_transport);
    socket.register_get_direct_mem_ptr(this, &renode_memory_transport::get_direct_mem_ptr);
    socket.register_transport_dbg(this, &renode_memory_transport::transport_dbg);
  }

  void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
    uint64_t addr = trans.get_address();
    void *ptr = trans.get_data_ptr();
    size_t len = trans.get_data_length();

    if (trans.is_write()) {
      renode_write_bytes_to_bus(addr, ptr, len);
    } else {
      renode_read_bytes_from_bus(addr, ptr, len);
    }

    trans.set_dmi_allowed(true);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data) {
    auto region = renode_get_direct_mem_ptr(trans.get_address());
    if (!region) {
      return false;
    }
    auto [dmi_ptr, start, end] = *region;

    dmi_data.set_dmi_ptr(dmi_ptr);
    dmi_data.set_start_address(start);
    dmi_data.set_end_address(end);
    dmi_data.allow_read_write();
    return true;
  }

  unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
    return 0;
  }
};

struct top : sc_core::sc_module, IRenodeBridge {
  renode_memory_transport *memory;
  dmi_test *dut;
  sc_core::sc_signal<bool> test_begin;

  top(sc_core::sc_module_name name, renode_memory_transport *m, dmi_test *d) : sc_module(name), memory(m), dut(d) {
    dut->bus_initiator_socket(memory->socket);
    dut->test_begin(test_begin);
  }

  void reset() override {
    // Not supported
  }

  tlm::tlm_fw_transport_if<> *tlm_route(std::uint64_t) override {
    // All memory accesses are served by Renode
    return nullptr;
  }

  void gpio_port_write(int number, bool value) override {
    if (number == 1) {
      test_begin.write(value);
    }
  }
};

static IRenodeBridge *create_hierarchy(void) {
  auto *memory = new renode_memory_transport("m_renode_memory_transport");
  auto *dut = new dmi_test("m_dmi_test");
  return new top("m_top", memory, dut);
}

static struct RegisterHierarchy {
  RegisterHierarchy() {
    register_renode_bridge_factory(create_hierarchy);
  }
} autoreg;

int sc_main(int argc, char *argv[]) {
  return 0;
}
