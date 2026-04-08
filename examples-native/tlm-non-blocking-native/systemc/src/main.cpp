#include <systemc>
#include <tlm>
#include "renode_bridge_native.h"
#include "peripheral.h"

struct top : sc_core::sc_module, IRenodeBridge, tlm::tlm_bw_transport_if<> {
  tlm::tlm_fw_transport_if<> *module;

  top(sc_core::sc_module_name name, tlm::tlm_fw_transport_if<> *m) : sc_module(name), module(m) {
    this->nb_done = false;
  }

  void invalidate_direct_mem_ptr(sc_dt::uint64, sc_dt::uint64) {
    // DMIs not supported on this initiator
  }

  tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& payload, tlm::tlm_phase& phase, sc_core::sc_time&) {
    if (phase == tlm::END_REQ) {
      return tlm::TLM_ACCEPTED;
    }

    if (phase == tlm::BEGIN_RESP) {
      nb_done = true;
      phase = tlm::END_RESP;

      return tlm::TLM_UPDATED;
    }

    return tlm::TLM_COMPLETED;
  }

  void reset() override {
    // reset not supported for this DUT
  }

  tlm::tlm_fw_transport_if<> *tlm_route(std::uint64_t) override {
    // route all traffic to the peripheral module
    return module;
  }

  void gpio_port_write(int number, bool value) override {
    // gpio not supported for this DUT
  }
};

static IRenodeBridge *create_hierarchy(void) {
  auto *dut = new peripheral("m_peripheral");
  auto *initiator = new top("m_top", dut);
  dut->initiator = initiator;

  return initiator;
}

static struct RegisterHierarchy {
  RegisterHierarchy() {
    register_renode_bridge_factory(create_hierarchy);
  }
} autoreg;

int sc_main(int, char **argv) {
  return 0;
}
