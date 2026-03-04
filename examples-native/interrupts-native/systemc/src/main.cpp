#include <systemc>
#include <tlm>
#include "renode_bridge_native.h"
#include "port_interrupter.h"

struct top : sc_core::sc_module, IRenodeBridge {
  tlm::tlm_fw_transport_if<> *interruptor_0, *interruptor_1;
  sc_core::sc_signal<bool> gpio_port[2];

  SC_HAS_PROCESS(top);
  top(sc_core::sc_module_name name, tlm::tlm_fw_transport_if<> *interruptor_0, tlm::tlm_fw_transport_if<> *interruptor_1) :
    sc_module(name), interruptor_0(interruptor_0), interruptor_1(interruptor_1) {
    auto *dut_0 = dynamic_cast<port_interrupter*>(interruptor_0);
    dut_0->irq_out(gpio_port[0]);

    auto *dut_1 = dynamic_cast<port_interrupter*>(interruptor_1);
    dut_1->irq_out(gpio_port[1]);
    
    SC_METHOD(gpio_changed);
    sensitive << gpio_port[0] << gpio_port[1];
  }

  void reset() override {
    // reset not supported for this DUT
  }

  tlm::tlm_fw_transport_if<> *tlm_route(std::uint64_t offset) override {
    switch (offset) {
    case 0x0:
      return interruptor_0;
      break;
    case 0x100000:
      return interruptor_1;
      break;
    default:
      return interruptor_0;
      break;
    }
  }

  void gpio_port_write(int number, bool value) override {
    // GPIO write not supported for this DUT
  }

  void gpio_changed() {
    for (int i = 0; i < 2; i++) {
      renode_gpio_update(i, gpio_port[i].read());
    }
  }
};

static IRenodeBridge *create_hierarchy(void) {
  auto *interruptor_0 = new port_interrupter("m_port_interrupter_0", 1);
  auto *interruptor_1 = new port_interrupter("m_port_interrupter_1", 3);
  return new top("m_top", interruptor_0, interruptor_1);
}

static struct RegisterHierarchy {
  RegisterHierarchy() {
    register_renode_bridge_factory(create_hierarchy);
  }
} autoreg;

int sc_main(int, char **argv) {
  return 0;
}
