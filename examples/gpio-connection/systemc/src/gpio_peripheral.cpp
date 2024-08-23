#include "gpio_peripheral.h"

SC_HAS_PROCESS(gpio_peripheral);
gpio_peripheral::gpio_peripheral(sc_core::sc_module_name name)
    : sc_module(name), gpio_received(false) {
  SC_METHOD(on_gpio_in);
  sensitive << gpio_in;
  dont_initialize();

  bus_target_socket.register_b_transport(this,
                                         &gpio_peripheral::receive_b_transport);
}

void gpio_peripheral::on_gpio_in() {
  printf("GPIO signal received in SystemC\n");
  gpio_received = true;
}

void gpio_peripheral::receive_b_transport(tlm::tlm_generic_payload &trans,
                                          sc_core::sc_time &delay) {
  assert(trans.is_read());
  uint32_t *value = (uint32_t *)trans.get_data_ptr();
  *value = gpio_received ? 1 : 0;
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
