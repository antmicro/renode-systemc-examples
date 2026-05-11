#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <cassert>
#include <cstring>
#include <vector>

#include "dmi_test.h"
#include "renode_bridge_native.h"

enum class RenodeGpio : int {
  test_done = 2,
  unexpected_io_access = 3,
};

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

struct test_state {
  void reset() {
    memory_done = false;
    cpu_done = false;
    renode_gpio_update(static_cast<int>(RenodeGpio::test_done), 0);
  }

  void mark_memory_done() {
    memory_done = true;
    update();
  }

  void mark_cpu_done() {
    cpu_done = true;
    update();
  }

private:
  void update() {
    if(memory_done && cpu_done) {
      renode_gpio_update(static_cast<int>(RenodeGpio::test_done), 1);
    }
  }

  bool memory_done = false;
  bool cpu_done = false;
};

struct systemc_cpu_target : sc_core::sc_module, tlm::tlm_fw_transport_if<> {

  systemc_cpu_target(sc_core::sc_module_name name, test_state &state)
    : sc_module(name), state(state), memory(MemorySize, 0) {}

  void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
    const auto address = trans.get_address();
    const auto length = trans.get_data_length();

    if(handle_done_register(trans, address, length)) {
      return;
    }

    if(address + length > MemorySize) {
      trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      return;
    }

    // Expected accesses: byte writes by AssembleBlock. They already set up the mapping for CPU
    // instruction fetch over DMI, so there is no need to expect any reads.
    if(!(trans.is_write() && length == 1)) {
      renode_gpio_update(static_cast<int>(RenodeGpio::unexpected_io_access), 1);
    }

    if(trans.is_write()) {
      std::memcpy(memory.data() + address, trans.get_data_ptr(), length);
    } else {
      std::memcpy(trans.get_data_ptr(), memory.data() + address, length);
    }

    trans.set_dmi_allowed(true);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data) {
    if(trans.get_address() >= MemorySize) {
      return false;
    }
    dmi_data.set_dmi_ptr(memory.data());
    dmi_data.set_start_address(0);
    dmi_data.set_end_address(memory.size() - 1);
    dmi_data.allow_read_write();
    return true;
  }

  tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &, sc_core::sc_time &delay) {
    return tlm::TLM_COMPLETED;
  }

  unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
    return 0;
  }

  static const std::uint64_t MemorySize = 0x1000;
  static const std::uint64_t DoneOffset = 0x1000; // after memory
  static const std::uint64_t DoneSize = sizeof(std::uint32_t);

private:
  bool handle_done_register(tlm::tlm_generic_payload &trans, std::size_t address, std::size_t length) {
    if(!(address >= DoneOffset && address + length <= DoneOffset + DoneSize)) {
      return false;
    }

    if(trans.is_write()) {
      state.mark_cpu_done();
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }
    return true;
  }

  test_state &state;
  std::vector<std::uint8_t> memory;
};

struct top : sc_core::sc_module, IRenodeBridge {
  renode_memory_transport *memory;
  systemc_cpu_target *cpu_target;
  dmi_test *dut;
  sc_core::sc_signal<bool> test_begin;
  test_state state;

  top(sc_core::sc_module_name name) : sc_module(name), test_begin("test_begin") {
    memory = new renode_memory_transport("m_renode_memory_transport");
    cpu_target = new systemc_cpu_target("m_systemc_cpu_target", state);
    dut = new dmi_test("m_dmi_test",
                       [this]() { state.reset(); },
                       [this]() { state.mark_memory_done(); });
    dut->bus_initiator_socket(memory->socket);
    dut->test_begin(test_begin);
  }

  void reset() override {
    // Not supported
  }

  tlm::tlm_fw_transport_if<> *tlm_route(std::uint64_t offset) override {
    if(offset < systemc_cpu_target::MemorySize
       || (offset >= systemc_cpu_target::DoneOffset && offset < systemc_cpu_target::DoneOffset + systemc_cpu_target::DoneSize)) {
      return cpu_target;
    }
    return nullptr;
  }

  void gpio_port_write(int number, bool value) override {
    if (number == 1) {
      test_begin.write(value);
    }
  }
};

static IRenodeBridge *create_hierarchy(void) {
  return new top("m_top");
}

static struct RegisterHierarchy {
  RegisterHierarchy() {
    register_renode_bridge_factory(create_hierarchy);
  }
} autoreg;

int sc_main(int argc, char *argv[]) {
  return 0;
}
