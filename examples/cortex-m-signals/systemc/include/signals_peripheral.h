#pragma once

#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

class signals_peripheral : public sc_core::sc_module {
public:
  signals_peripheral(sc_core::sc_module_name name);

  tlm_utils::simple_target_socket<signals_peripheral> bus_target_socket;

  sc_core::sc_out<bool> out_non_maskable_interrupt; // nmi_exp
  sc_core::sc_out<bool> out_core_reset_in;          // core_reset_in
  sc_core::sc_out<bool> out_cpu_wait;               // cpu_wait
  sc_core::sc_out<bool> out_power_on_reset;         // m55_poreset_n

  sc_core::sc_in<bool> in_system_reset_request; // O_sysreset_req
  sc_core::sc_in<bool> in_sleeping;             // O_sleeping
  sc_core::sc_in<bool> in_sleep_deep;           // O_sleep_deep

private:
  void receive_b_transport(tlm::tlm_generic_payload &trans,
                           sc_core::sc_time &delay);
  void on_gpio_in();
  void handle_read(tlm::tlm_generic_payload &trans);
  void handle_write(tlm::tlm_generic_payload &trans);
};

enum Signal {
  NonMaskableInterrupt = 0,           // nmi_exp
  CoreResetIn = 1,                    // core_reset_in
  CpuWait = 2,                        // cpu_wait
  InitNonSecureVectorTableOffset = 3, // init_ns_vtor
  InitSecureVectorTableOffset = 4,    // m55_initsvtor
  PowerOnReset = 5,                   // m55_poreset_n
  SystemResetRequest = 6,             // O_sysreset_req
  Sleeping = 7,                       // O_sleeping
  SleepDeep = 8,                      // O_sleep_deep
};

constexpr uint64_t TRIGGER_SIGNAL_OFFSET = 0x100;
