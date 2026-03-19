#include <systemc>
#include <tlm>
#include "renode_bridge_native.h"
#include "timekeeper.h"

struct top : sc_core::sc_module, IRenodeBridge {
  tlm::tlm_fw_transport_if<> *module;

  top(sc_core::sc_module_name name, tlm::tlm_fw_transport_if<> *m) : sc_module(name), module(m) {
    // intentionally empty
  }

  void reset() override {
    // reset not supported for this DUT
  }

  tlm::tlm_fw_transport_if<> *tlm_route(std::uint64_t) override {
    // route all traffic to timekeeper
    return module;
  }

  void gpio_port_write(int number, bool value) override {
    // gpio not supported for this DUT
  }
};

static IRenodeBridge *create_hierarchy(void) {
  auto *dut = new timekeeper("m_timekeeper");
  return new top("m_top", dut);
}

static struct RegisterHierarchy {
  RegisterHierarchy() {
    register_renode_bridge_factory(create_hierarchy);
  }
} autoreg;

int sc_main(int, char **argv) {
  return 0;
}
