// Copyright (c) 2024-2026 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <signals_peripheral.h>

SC_HAS_PROCESS(signals_peripheral);
signals_peripheral::signals_peripheral(sc_core::sc_module_name name)
    : sc_module(name) {
  sensitive << in_system_reset_request << in_sleeping << in_sleep_deep;

  out_non_maskable_interrupt.initialize(false);
  out_core_reset_in.initialize(false);
  out_cpu_wait.initialize(false);
  out_power_on_reset.initialize(false);

  bus_target_socket.register_b_transport(
      this, &signals_peripheral::receive_b_transport);
}

void signals_peripheral::receive_b_transport(tlm::tlm_generic_payload &payload,
                                             sc_core::sc_time &delay) {
  // Yield to let the signals update first.
  wait(sc_core::SC_ZERO_TIME);

  if (payload.is_read()) {
    handle_read(payload);
  } else {
    handle_write(payload);
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

void signals_peripheral::handle_read(tlm::tlm_generic_payload &payload) {
  assert(payload.is_read());
  auto *is_set = (bool *)payload.get_data_ptr();
  auto offset = payload.get_address();
  std::cout << "Info: got read at offset 0x" << std::hex
            << payload.get_address() << "\n";

  auto signal = static_cast<Signal>(offset);
  switch (signal) {
  case Signal::SystemResetRequest:
    *is_set = in_system_reset_request.read();
    break;
  case Signal::Sleeping:
    *is_set = in_sleeping.read();
    break;
  case Signal::SleepDeep:
    *is_set = in_sleep_deep.read();
    break;
  default:
    std::cerr << "Error: Unhandled signal: " << static_cast<int>(signal)
              << std::endl;
    std::abort();
  }
}

void signals_peripheral::handle_write(tlm::tlm_generic_payload &payload) {
  assert(payload.is_write());
  auto address = payload.get_address();
  if (address != TRIGGER_SIGNAL_OFFSET) {
    std::cout << "Warning: Unhandled write to unknown offset\n";
    return;
  }

  auto request = *(Signal *)payload.get_data_ptr();
  std::cout << "Info: Will trigger signal " << request << "\n";

  if (request >= Signal::NvicIrqsStart && request <= Signal::NvicIrqsEnd) {
    std::cout << "Info: Signal is an NVIC IRQ\n";
    out_nvic_irqs[request].write(true);
    return;
  }

  switch (request) {
  case Signal::NonMaskableInterrupt:
    out_non_maskable_interrupt.write(true);
    break;
  case Signal::CoreResetIn:
    out_core_reset_in.write(true);
    break;
  case Signal::CpuWait:
    out_cpu_wait.write(true);
    break;
  case Signal::PowerOnReset:
    out_power_on_reset.write(true);
    break;
  default:
    std::cerr << "Error: Unhandled signal: " << static_cast<int>(request)
              << std::endl;
    std::abort();
  }
}
