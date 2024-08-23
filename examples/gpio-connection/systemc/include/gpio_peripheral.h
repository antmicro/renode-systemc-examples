#pragma once

#include <memory>

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

class gpio_peripheral : public sc_core::sc_module {
public:
  gpio_peripheral(sc_core::sc_module_name name);

  tlm_utils::simple_target_socket<gpio_peripheral> bus_target_socket;
  sc_core::sc_in<bool> gpio_in;

private:
  void receive_b_transport(tlm::tlm_generic_payload &trans,
                           sc_core::sc_time &delay);
  void on_gpio_in();

  bool gpio_received;
};
