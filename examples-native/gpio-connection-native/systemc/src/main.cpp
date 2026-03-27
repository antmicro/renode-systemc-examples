#include <systemc>
#include <tlm>
#include "renode_bridge_native.h"
#include "gpio_peripheral.h"

struct top : sc_core::sc_module, IRenodeBridge {
  tlm::tlm_fw_transport_if<> *module;
  sc_core::sc_signal<bool> gpio_port;

  top(sc_core::sc_module_name name, tlm::tlm_fw_transport_if<> *m) : sc_module(name), module(m) {
    // bind gpio signals
    auto *dut = dynamic_cast<gpio_peripheral*>(module);
    dut->gpio_in(gpio_port);
  }

  void reset() override {
    // reset not supported for this DUT
  }

  tlm::tlm_fw_transport_if<> *tlm_route(std::uint64_t offset) override {
    // route all traffic to this DUT
    return module;
  }

  void gpio_port_write(int number, bool value) override {
    gpio_port.write(value);
  }
};

static IRenodeBridge *create_hierarchy(void) {
  auto *dut = new gpio_peripheral("m_gpio_peripheral");
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
